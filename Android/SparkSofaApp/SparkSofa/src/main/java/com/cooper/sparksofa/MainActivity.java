package com.cooper.sparksofa;

import android.content.Intent;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

public class MainActivity extends ActionBarActivity implements WebUtility.Callback
{
    private TextView tvDebug;
    private Spinner spSofaSelector;
    private Button butMoveToUp;
    private Button butMoveToFeet;
    private Button butMoveToFlat;
    private Button butManUp;
    private Button butManDown;
    private static final int RESULT_SETTINGS = 1;
    private Menu menu;
    private WebUtility webUtility = new WebUtility(this);

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        // Set the preference defaults
        PreferenceManager.setDefaultValues(this, R.xml.settings, false);

        setContentView(R.layout.activity_main);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        if (toolbar != null) {
            setSupportActionBar(toolbar);
        }

        tvDebug = (TextView) findViewById(R.id.debug_text_view);
        spSofaSelector = (Spinner) findViewById(R.id.spinner_sofa_selector);

        ArrayAdapter<String> spinnerArrayAdapter = new ArrayAdapter<>(this, R.layout.spinner_item_text, getResources().getStringArray(R.array.sofa_arrays));
        spSofaSelector.setAdapter(spinnerArrayAdapter);

        butMoveToUp = (Button) findViewById(R.id.moveToUp);
        butMoveToFeet = (Button) findViewById(R.id.moveToFeet);
        butMoveToFlat = (Button) findViewById(R.id.moveToFlat);
        butManUp = (Button) findViewById(R.id.manUp);
        butManDown = (Button) findViewById(R.id.manDown);

        butMoveToUp.setOnClickListener(moveHandler);
        butMoveToFeet.setOnClickListener(moveHandler);
        butMoveToFlat.setOnClickListener(moveHandler);

        butManUp.setOnTouchListener(manualTouchHandler);
        butManDown.setOnTouchListener(manualTouchHandler);

        webUtility.callback = this;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        webUtility.getModeValue();

        this.menu = menu;
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        switch (item.getItemId())
        {

            case R.id.action_settings:
                Intent i = new Intent(this, SettingsActivity.class);
                startActivityForResult(i, RESULT_SETTINGS);
                break;

            case R.id.action_debug:
                if (tvDebug.getVisibility() == View.GONE)
                    tvDebug.setVisibility(View.VISIBLE);
                else
                    tvDebug.setVisibility(View.GONE);
                break;

//            case R.id.action_stop_all:
//                postWebPage(getBaseURL() + "/setMode", "stop");
//                break;

            case R.id.action_parental_mode:
                webUtility.toggleParentalMode();
                break;

            case R.id.action_crazy_mode:
                webUtility.toggleCrazyMode();
                break;
        }

        return true;
    }

    View.OnClickListener moveHandler = new View.OnClickListener()
    {
        public void onClick(View view)
        {
            switch (view.getId())
            {
                case R.id.moveToUp:
                    webUtility.moveTo(spSofaSelector.getSelectedItemPosition(), WebUtility.SeatPosition.Upright);
                    break;

                case R.id.moveToFeet:
                    webUtility.moveTo(spSofaSelector.getSelectedItemPosition(), WebUtility.SeatPosition.FeetUp);
                    break;

                case R.id.moveToFlat:
                    webUtility.moveTo(spSofaSelector.getSelectedItemPosition(), WebUtility.SeatPosition.Flat);
                    break;
            }
        }
    };

    View.OnTouchListener manualTouchHandler = new View.OnTouchListener()
    {
        @Override
        public boolean onTouch(View view, MotionEvent event)
        {

            if (event.getAction() == MotionEvent.ACTION_DOWN)
            {
                switch (view.getId())
                {
                    case R.id.manUp:
                        webUtility.pressUpButton(spSofaSelector.getSelectedItemPosition());
                        break;

                    case R.id.manDown:
                        webUtility.pressDownButton(spSofaSelector.getSelectedItemPosition());
                        break;
                }
            }
            else if (event.getAction() == MotionEvent.ACTION_UP)
                webUtility.releaseButton(spSofaSelector.getSelectedItemPosition());

            return false;
        }
    };

    public void modeChanged(int modeValue)
    {
        boolean[] values = new boolean[2];
        values[0] = modeValue % 2 != 0;
        modeValue /= 2;
        values[1] = modeValue % 2 != 0;

        MenuItem item = menu.findItem(R.id.action_parental_mode);
        item.setChecked(values[0]);
        item = menu.findItem(R.id.action_crazy_mode);
        item.setChecked(values[1]);
    }

    public void statusReceived(String status)
    {
        tvDebug.setText(status);
    }
}