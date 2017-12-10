//package net.braingang.perkyblue;
package parallel.park;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.hardware.SensorManager;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.OrientationEventListener;
import android.view.View;
import android.widget.Button;
import android.view.MotionEvent;

import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.UUID;

/*
import static net.braingang.perkyblue.MainActivity.MOVE.BACKWARD;
import static net.braingang.perkyblue.MainActivity.MOVE.FORWARD;
import static net.braingang.perkyblue.MainActivity.MOVE.PARK;
import static net.braingang.perkyblue.MainActivity._orientation;
import static net.braingang.perkyblue.MainActivity._socket;
import net.braingang.perkyblue.MainActivity.*;
*/
import parallel.park.MainActivity.MOVE;


public class DriveActivity extends AppCompatActivity{

    private OrientationEventListener _OrientationListener;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_drive);

        /*
        findViewById(R.id.buttonToggleForward).setOnTouchListener(this);
        findViewById(R.id.buttonToggleBackward).setOnTouchListener(this);
        findViewById(R.id.buttonTogglePark).setOnTouchListener(this);
        */

        Button forward = findViewById(R.id.button);
        Button backward = findViewById(R.id.button2);
        Button park = findViewById(R.id.button3);

        forward.setOnTouchListener(handleTouch);
        backward.setOnTouchListener(handleTouch);
        park.setOnTouchListener(handleTouch);


        _OrientationListener = new OrientationEventListener(this,
                SensorManager.SENSOR_DELAY_NORMAL) {

            @Override
            public void onOrientationChanged(int orientation) {
                Log.v("DEBUG_TAG",
                        "Orientation changed to " + orientation);

                //_orientation = orientation;



            }


        };

        if (_OrientationListener.canDetectOrientation()) {
            Log.v("DEBUG_TAG", "Can detect orientation");
            _OrientationListener.enable();
        } else {
            Log.v("DEBUG_TAG", "Cannot detect orientation");
            _OrientationListener.disable();
        }





/*
        park.setOnTouchListener(new View.OnTouchListener() {

            private Handler mHandler;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch(event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        if (mHandler != null) return true;
                        mHandler = new Handler();
                        mHandler.postDelayed(new WriteRead(_socket, FORWARD), 500);

                        break;

                    case MotionEvent.ACTION_UP:
                        mHandler.postDelayed(new WriteRead((_socket,PARK), 500);
                }
                return false;
            }



        });//*/



}

/*

    private Handler mHandler;

    @Override public boolean onTouch(View v, MotionEvent event) {
        switch(event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                if (mHandler != null) return true;
                mHandler = new Handler();
                mHandler.postDelayed(new WriteRead(_socket, FORWARD), 500);
                break;
            case MotionEvent.ACTION_UP:
                mHandler.postDelayed(new WriteRead(_socket, PARK), 500);
        }
        return false;
    }
*/


    private View.OnTouchListener handleTouch = new View.OnTouchListener() {

        private Handler mHandler;

        @Override public boolean onTouch(View v, MotionEvent event) {
            switch(event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    if (mHandler != null) return true;
                    mHandler = new Handler();
                    //mHandler.postDelayed(new WriteRead(_socket, FORWARD), 500);
                    break;
                case MotionEvent.ACTION_UP:
                    //mHandler.postDelayed(new WriteRead(_socket, PARK), 500);
            }
            return false;
        }


    };


/*
    @Override
    public void onClick(View view) {
        WriteRead writeRead;
        switch (view.getId()) {
            // Create Button functions here
            case R.id.buttonToggleForward:
                writeRead = new WriteRead(_socket, FORWARD);
                new Thread(writeRead).start();
                break;
            case R.id.buttonToggleBackward:
                writeRead = new WriteRead(_socket, BACKWARD);
                new Thread(writeRead).start();
                break;
            case R.id.buttonTogglePark:
                writeRead = new WriteRead(_socket, PARK);
                new Thread(writeRead).start();
                break;

            default:
                Log.i("LOG_TAG", "unknown click event");
        }
    }*/

 /*   private void bluetoothConnect(BluetoothDevice target) {
        MainActivity._selectedDevice = target;
        MainActivity._tvBtAddress.setText(target.getAddress());
        MainActivity._tvBtName.setText(target.getName());

        try {
            _socket = target.createRfcommSocketToServiceRecord(UUID.fromString(MainActivity.SERIAL_SERVICE));
            _socket.connect();

            toggleButton(true);
            MainActivity.updateState(MainActivity.BT_STATE.CONNECTED_STATE);
            Intent intent = new Intent(this, DriveActivity.class);
            startActivity(intent);
        } catch(Exception exception) {
            MainActivity.bluetoothClose();
            exception.printStackTrace();
            MainActivity.updateState(MainActivity.BT_STATE.FAILURE_STATE);
        }
    }


    private void toggleButton(boolean flag) {
        findViewById(R.id.buttonToggleForward).setEnabled(flag);
        findViewById(R.id.buttonToggleBackward).setEnabled(flag);
        findViewById(R.id.buttonTogglePark).setEnabled(flag);
    }*/
}


/*
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

    /*public String getResponse() {
        return _stringBuilder.toString();
    }*//*

    public void run() {
        try {
            //_reader = new InputStreamReader(_socket.getInputStream(), "UTF-8");
            _writer = new OutputStreamWriter(_socket.getOutputStream(), "UTF-8");

            int diff = 270 - _orientation;
            int duty = Math.abs(diff);
            if (duty < 10) duty = 10;

            // Write messages to send to BBB here
            switch(_move) {
                case FORWARD:
                    Log.i(LOG_TAG, "send forward");
                    if (diff < 10) { // Go right
                        _writer.write(String.format("4,%d,1,",duty));
                    } else if (diff > 10) {
                        _writer.write(String.format("2,%d,1,",duty));
                    } else {
                        _writer.write("2,20,1,");
                    }

                    //_writer.write("0,20,2");
                    _writer.flush();
                    break;
                case BACKWARD:
                    Log.i(LOG_TAG, "send backward");
                    if (diff < 10) { // Go right
                        _writer.write(String.format("5,%d,1,",duty));
                    } else if (diff > 10) {
                        _writer.write(String.format("3,%d,1,",duty));
                    } else {
                        _writer.write("1,20,1,");
                    }
                    //_writer.write("1,20,2");
                    _writer.flush();
                    break;
                case PARK:
                    Log.i(LOG_TAG, "send park");
                    _writer.write("7,00,0, ");
                    _writer.flush();
                    break;
            }*/

            /*
            final char[] buffer = new char[8];
            while (true) {
                int size = _reader.read(buffer);
                if (size < 0) {
                    break;
                } else {
                    _stringBuilder.append(buffer, 0, size);
                }
            }
            *//*
        } catch (Exception exception) {
            exception.printStackTrace();
        }
    }
}*/
