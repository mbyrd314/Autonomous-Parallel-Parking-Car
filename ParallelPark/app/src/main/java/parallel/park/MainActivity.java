package parallel.park;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.SensorManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.OrientationEventListener;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;


import java.io.OutputStreamWriter;
import java.io.Writer;

import java.util.ArrayList;
import java.util.UUID;

import static parallel.park.MainActivity.MOVE.FORWARD;
import static parallel.park.MainActivity.MOVE.BACKWARD;
import static parallel.park.MainActivity.MOVE.PARK;
import static parallel.park.MainActivity.MOVE.STOP;
import static parallel.park.MainActivity._orientation;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    public final String LOG_TAG = getClass().getName();

    public final String SERIAL_SERVICE = "00001101-0000-1000-8000-00805F9B34FB";

    public enum MOVE {FORWARD, BACKWARD, PARK, STOP};
    public enum BT_STATE {UNKNOWN_STATE, CONNECTED_STATE, DISCOVERY_FINISH_STATE, DISCOVERY_START_STATE, FAILURE_STATE, NULL_ADAPTER};

    private ArrayList<BluetoothDevice> _deviceList = new ArrayList<BluetoothDevice>();
    private BluetoothDevice _selectedDevice = null;

    public static BluetoothSocket _socket = null;

    private ListView _deviceListView;
    private BtArrayAdapter _btArrayAdapter;

    private BluetoothAdapter _bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

    public TextView _tvBtAddress;
    public TextView _tvBtName;
    public TextView _tvBtState;

    // Integer representing the phone's angle
    public static int _orientation;

    // This monitors the phone's orientation
    private OrientationEventListener _OrientationListener;

    private BroadcastReceiver receiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();

            if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                BluetoothDevice temp = (BluetoothDevice) intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                if (_deviceList.contains(temp)) {
                    Log.i(LOG_TAG, "skipping:" + temp.getName() + ":" + temp.getAddress());
                } else {
                    _deviceList.add(temp);
                }
            } else if (BluetoothAdapter.ACTION_DISCOVERY_STARTED.equals(intent.getAction())) {
                updateState(BT_STATE.DISCOVERY_START_STATE);
            } else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(intent.getAction())) {
                updateState(BT_STATE.DISCOVERY_FINISH_STATE);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        toggleButton(false);
        startCancelButton(true);

        // Initializing the discovery start and cancel buttons
        findViewById(R.id.buttonDiscoveryCancel).setOnClickListener(this);
        findViewById(R.id.buttonDiscoveryStart).setOnClickListener(this);

        _deviceListView = (ListView) findViewById(R.id.lvBtDevice);

        _tvBtAddress = (TextView) findViewById(R.id.tvBtAddress);
        _tvBtName = (TextView) findViewById(R.id.tvBtName);
        _tvBtState = (TextView) findViewById(R.id.tvBtState);

        // Initializing the orientation listener
        _OrientationListener = new OrientationEventListener(this,
                SensorManager.SENSOR_DELAY_NORMAL) {

            // This function changes the variable _orientation to the current orientation
            @Override
            public void onOrientationChanged(int orientation) {
                Log.v("DEBUG_TAG",
                        "Orientation changed to " + orientation);

                _orientation = orientation;



            }


        };

        if (_OrientationListener.canDetectOrientation()) {
            Log.v("DEBUG_TAG", "Can detect orientation");
            _OrientationListener.enable();
        } else {
            Log.v("DEBUG_TAG", "Cannot detect orientation");
            _OrientationListener.disable();
        }
    }


    @Override
    public void onClick(View view) {
        // Initializing so that the app can communicate with the BBB
        WriteRead writeRead;
        // Doing the correct action based on which button was pressed
        switch (view.getId()) {
            case R.id.buttonDiscoveryCancel:
                discoveryStop();
                break;
            case R.id.buttonDiscoveryStart:
                discoveryStart();
                break;
            case R.id.button:
                writeRead = new WriteRead(_socket, BACKWARD);
                new Thread(writeRead).start();
                break;
            case R.id.button2:
                writeRead = new WriteRead(_socket, FORWARD);
                new Thread(writeRead).start();
                break;
            case R.id.button3:
                writeRead = new WriteRead(_socket, PARK);
                new Thread(writeRead).start();
                break;
            case R.id.button4:
                writeRead = new WriteRead(_socket, STOP);
                new Thread(writeRead).start();
                break;

            default:
                Log.i(LOG_TAG, "unknown click event");
        }
    }

    private void discoveryStart() {
        Log.d(LOG_TAG, "discoveryStart");

        if (_bluetoothAdapter == null) {
            updateState(BT_STATE.NULL_ADAPTER);
            return;
        }

        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothDevice.ACTION_FOUND);
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        registerReceiver(receiver, filter);

        _bluetoothAdapter.startDiscovery();
    }

    private void discoveryStop() {
        Log.d(LOG_TAG, "discoveryStop");

        if (_bluetoothAdapter == null) {
            Log.i(LOG_TAG, "unable to run w/null BT adapter");
            return;
        }

        _bluetoothAdapter.cancelDiscovery();
    }

    public void bluetoothClose() {
        if (_socket != null) {
            try {
                _socket.close();
            } catch (Exception exception) {
                // empty
            }
        }

        _socket = null;
    }

    private void bluetoothConnect(BluetoothDevice target) {
        _selectedDevice = target;
        _tvBtAddress.setText(target.getAddress());
        _tvBtName.setText(target.getName());

        try {
            _socket = target.createRfcommSocketToServiceRecord(UUID.fromString(SERIAL_SERVICE));
            _socket.connect();

            toggleButton(true);
            updateState(BT_STATE.CONNECTED_STATE);
            // When discovery finishes, switch to the second layout
            // This layout has the forward, backward, park, and stop buttons
            setContentView(R.layout.activity_drive);

        } catch(Exception exception) {
            bluetoothClose();
            exception.printStackTrace();
            updateState(BT_STATE.FAILURE_STATE);
        }
    }

    private void updateState(BT_STATE target) {
        switch(target) {
            case CONNECTED_STATE:
                toggleButton(true);
                _tvBtState.setText(getString(R.string.label_bt_state_connected));
                break;
            case DISCOVERY_FINISH_STATE:
                _btArrayAdapter = new BtArrayAdapter(getBaseContext(), _deviceList);
                _deviceListView.setAdapter(_btArrayAdapter);

                _deviceListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                    public void onItemClick(AdapterView<?> adapter, View view, int position, long id) {
                        bluetoothConnect((BluetoothDevice) adapter.getItemAtPosition(position));
                    }
                });

                _tvBtState.setText(getString(R.string.label_bt_state_discovery_finish));
                startCancelButton(true);
                break;
            case DISCOVERY_START_STATE:
                _deviceList.clear();
                _tvBtState.setText(getString(R.string.label_bt_state_discovery_start));
                startCancelButton(false);
                break;
            case FAILURE_STATE:
                bluetoothClose();
                toggleButton(false);
                startCancelButton(true);
                _tvBtState.setText(getString(R.string.label_bt_state_failure));
                break;
            case NULL_ADAPTER:
                toggleButton(false);
                startCancelButton(true);
                _tvBtState.setText(getString(R.string.label_null_adapter));
                break;
            case UNKNOWN_STATE:
                toggleButton(false);
                startCancelButton(true);
                _tvBtState.setText(getString(R.string.label_bt_state_unknown));
                break;
        }
    }

    public void toggleButton(boolean flag) {
        /*
        findViewById(R.id.buttonToggleForward).setEnabled(flag);
        findViewById(R.id.buttonToggleBackward).setEnabled(flag);
        findViewById(R.id.buttonTogglePark).setEnabled(flag);
        */
    }

    private void startCancelButton(boolean flag) {
        if (flag) {
            findViewById(R.id.buttonDiscoveryCancel).setEnabled(false);
            findViewById(R.id.buttonDiscoveryStart).setEnabled(true);
        } else {
            findViewById(R.id.buttonDiscoveryCancel).setEnabled(true);
            findViewById(R.id.buttonDiscoveryStart).setEnabled(false);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        _OrientationListener.disable();
    }
}


class WriteRead implements Runnable {
    public final String LOG_TAG = getClass().getName();

    private final MainActivity.MOVE _move;
    private final BluetoothSocket _socket;

    //private Reader _reader;
    private Writer _writer;

    private final StringBuilder _stringBuilder = new StringBuilder();

    WriteRead(BluetoothSocket socket, MainActivity.MOVE move) {
        _socket = socket;
        _move = move;
    }

    public void run() {
        try {
            //_reader = new InputStreamReader(_socket.getInputStream(), "UTF-8");
            _writer = new OutputStreamWriter(_socket.getOutputStream(), "UTF-8");

            // Sets the duty cycle to be the absolute value of the angle of the phone from the vertical
            // If the angle < 30, the duty cycle is set to 30
            int diff = _orientation;
            if (diff > 180) diff = 360 - diff;
            int duty = Math.abs(diff);
            if (duty < 30) duty = 30;
            if (duty > 100) duty = 100;

            // Sends the correct instructions to the BBB
            // If the angle < 30, the car is moved either straight forward or straight backward at
            // a duty cycle of 30%
            // If the park command is desired, a message beginning with a 7 is sent, which ends the
            // User thread in the C process. This starts the parking protocol.
            // If the stop command is sent, a message beginning with a 6 is sent, which stops the car
            switch(_move) {
                case FORWARD:
                    Log.i(LOG_TAG, "send forward");
                    if (_orientation > 30 && _orientation < 180) { // Go right
                        _writer.write(String.format("4,%d,1,",duty));
                    } else if (_orientation > 180 && _orientation < 330) {
                        _writer.write(String.format("2,%d,1,",duty));
                    } else {
                        _writer.write("0,30,1,");
                    }

                    //_writer.write("0,30,2");
                    _writer.flush();
                    break;
                case BACKWARD:
                    Log.i(LOG_TAG, "send backward");
                    if (_orientation > 30 && _orientation < 180) { // Go right
                        _writer.write(String.format("5,%d,1,",duty));
                    } else if (_orientation > 180 && _orientation < 330) {
                        _writer.write(String.format("3,%d,1,",duty));
                    } else {
                        _writer.write("1,30,1,");
                    }
                    //_writer.write("1,30,2");
                    _writer.flush();
                    break;
                case PARK:
                    Log.i(LOG_TAG, "send park");
                    _writer.write("7,00,0, ");
                    _writer.flush();
                    break;
                case STOP:
                    _writer.write("6,00,0, ");
                    _writer.flush();
                    break;
            }


        } catch (Exception exception) {
            exception.printStackTrace();
        }
    }
}