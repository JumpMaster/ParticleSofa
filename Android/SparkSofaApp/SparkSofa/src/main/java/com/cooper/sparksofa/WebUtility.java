package com.cooper.sparksofa;

import android.app.Activity;
import android.preference.PreferenceManager;
import android.widget.Toast;

import com.loopj.android.http.AsyncHttpClient;
import com.loopj.android.http.AsyncHttpResponseHandler;
import com.loopj.android.http.RequestParams;

import org.apache.http.Header;
import org.json.JSONException;
import org.json.JSONObject;

public class WebUtility
{
    interface Callback {
        void modeChanged(int mode);
        void statusReceived(String status);
    }

    public enum SeatPosition
    {
        Upright,
        FeetUp,
        Flat
    }

    private AsyncHttpClient _client;
    private Activity _context;
    Callback callback;

    public WebUtility(Activity context)
    {
        _context = context;
        _client = new AsyncHttpClient();
    }

    private String getBaseURL()
    {
        return "https://api.spark.io/v1/devices/" + PreferenceManager.getDefaultSharedPreferences(_context).getString("prefSparkId", "NULL");
    }

    public void pressUpButton(int seat)
    {
        String stringUrl = getBaseURL();

//        String params = Integer.toString(seat) + ",";
        String params = Integer.toString(seat) + ",p,";
//        stringUrl += "/pressButt";
        stringUrl += "/moveManual";
        params += "up";

        postWebPage(stringUrl, params);
    }

    public void pressDownButton(int seat)
    {
        String stringUrl = getBaseURL();

//        String params = Integer.toString(seat) + ",";
        String params = Integer.toString(seat) + ",p,";
//        stringUrl += "/pressButt";
        stringUrl += "/moveManual";
        params += "down";

        postWebPage(stringUrl, params);
    }

    public void releaseButton(int seat)
    {
        String stringUrl = getBaseURL();

//        String params = Integer.toString(seat) + ",";
        String params = Integer.toString(seat) + ",r";
//        stringUrl += "/releaseButt";
        stringUrl += "/moveManual";
        postWebPage(stringUrl, params);
    }

    public void moveTo(int seat, SeatPosition position)
    {
        String stringUrl = getBaseURL() + "/moveTo";
        String params = Integer.toString(seat) + ",";

        switch (position) {
            case Upright:
                params += "upright";
                break;

            case FeetUp:
                params += "feetup";
                break;

            case Flat:
                params += "flat";
                break;
        }

        postWebPage(stringUrl, params);
    }

    public void getModeValue()
    {
        getWebPage(getBaseURL() + "/getModeValue");
    }

    public void toggleParentalMode()
    {
        postWebPage(getBaseURL() + "/setMode", "parental");
    }

    public void toggleCrazyMode()
    {
        postWebPage(getBaseURL() + "/setMode", "crazy");
    }

    private void postWebPage(final String url, String params)
    {
        RequestParams requestParams = new RequestParams();
        requestParams.put("access_token", PreferenceManager.getDefaultSharedPreferences(_context).getString("prefSparkToken", "NULL"));
        requestParams.put("params", params);
        _client.post(url, requestParams, new AsyncHttpResponseHandler()
        {
            @Override
            public void onSuccess(int statusCode, Header[] headers, byte[] responseBody)
            {
                String response = new String(responseBody);
                if (callback != null)
                    callback.statusReceived(String.valueOf(statusCode) + "\n" + response);

                if (url.toLowerCase().contains("setmode"))
                {
                    int modeValue;

                    try {
                        JSONObject mainObject = new JSONObject(response);
                        modeValue = mainObject.getInt("return_value");
                    } catch (JSONException e) {
                        modeValue = -1;
                        Toast.makeText(_context, "CRASH", Toast.LENGTH_SHORT).show();
                    }
                    if (modeValue >= 0 && callback != null)
                        callback.modeChanged(modeValue);
                }
            }

            @Override
            public void onFailure(int statusCode, Header[] headers, byte[] responseBody, Throwable error)
            {
                if (callback != null)
                    callback.statusReceived(String.valueOf(statusCode) + "\n" + String.valueOf(error.getMessage()));
            }
        });
    }

    private void getWebPage(final String url)
    {
        String urlWithToken = url + "?access_token=" + PreferenceManager.getDefaultSharedPreferences(_context).getString("prefSparkToken", "NULL");

        _client.get(urlWithToken, new AsyncHttpResponseHandler() {
            @Override
            public void onSuccess(int statusCode, Header[] headers, byte[] responseBody) {
                String response = new String(responseBody);

                if (callback != null)
                    callback.statusReceived(String.valueOf(statusCode) + "\n" + response);

                if (url.toLowerCase().contains("getmodevalue"))
                {
                    int modeValue;

                    try
                    {
                        JSONObject mainObject = new JSONObject(response);
                        modeValue = mainObject.getInt("result");
                    }
                    catch (JSONException e)
                    {
                        modeValue = -1;
                        Toast.makeText(_context, "CRASH", Toast.LENGTH_SHORT).show();
                    }
                    if (modeValue >= 0 && callback != null)
                        callback.modeChanged(modeValue);
                }
            }

            @Override
            public void onFailure(int statusCode, Header[] headers, byte[] responseBody, Throwable error)
            {
                if (callback != null)
                    callback.statusReceived(String.valueOf(statusCode) + "\n" + String.valueOf(error.getMessage()));
            }
        });
    }
}