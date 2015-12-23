package stoodx.trainswatchmobile;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;


public class MainActivity extends AppCompatActivity implements AdapterView.OnItemSelectedListener {

    public class Station {
        public String m_strID;
        public String m_strName;
    }

    private Station[] m_arrayStationsFrom;
    private Station[] m_arrayStationsTo;

    private Spinner m_spinnerFromA;
    private Spinner m_spinnerToA;
    private Spinner m_spinnerFrom;
    private Spinner m_spinnerTo;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //From
        m_spinnerFromA = (Spinner) findViewById(R.id.spinnerFromA);
        m_spinnerFromA.setOnItemSelectedListener(this);
        ArrayAdapter<CharSequence> adapterFromA = ArrayAdapter.createFromResource(this,
                R.array.spinnerFromA, android.R.layout.simple_spinner_item);
        adapterFromA.setDropDownViewResource(android.R.layout.simple_spinner_item);
        m_spinnerFromA.setAdapter(adapterFromA);

        m_spinnerFrom = (Spinner) findViewById(R.id.spinnerFrom);
        ArrayAdapter<CharSequence> adapterFrom = ArrayAdapter.createFromResource(this,
                R.array.spinnerFrom, android.R.layout.simple_spinner_item);
        adapterFrom.setDropDownViewResource(android.R.layout.simple_spinner_item);
        m_spinnerFrom.setAdapter(adapterFrom);


        //To
        m_spinnerToA = (Spinner) findViewById(R.id.spinnerToA);
        m_spinnerToA.setOnItemSelectedListener(this);
        ArrayAdapter<CharSequence> adapterToA = ArrayAdapter.createFromResource(this,
                R.array.spinnerToA, android.R.layout.simple_spinner_item);
        adapterToA.setDropDownViewResource(android.R.layout.simple_spinner_item);
        m_spinnerToA.setAdapter(adapterToA);
    }

    public void onItemSelected(AdapterView<?> parent, View view,
                               int pos, long id) {
        // An item was selected. You can retrieve the selected item using
        // parent.getItemAtPosition(pos)

        int iID = parent.getId();
        switch (iID) {
            case R.id.spinnerFromA:
                fillStations(m_spinnerFromA, m_spinnerFrom, m_arrayStationsFrom, pos);
                break;
            case R.id.spinnerToA:
                messageBox("Увага", "ToA " + m_spinnerToA.getItemAtPosition(pos));
                break;
            default:
                return;
        }
    }

    public void onNothingSelected(AdapterView<?> parent) {
        // Another interface callback
    }

    private void messageBox(String strTitle, String strMessage) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder
                .setTitle(strTitle)
                .setMessage(strMessage)
                .setIcon(android.R.drawable.ic_dialog_alert)
                .setPositiveButton("Ок", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });
        AlertDialog alert = builder.create();
        alert.show();
    }

    private boolean fillStations(Spinner spinnerA, Spinner spinner, Station[] arr, int pos) {
        //clear all lists
        arr = new Station[0];
        ArrayAdapter adap = (ArrayAdapter) spinner.getAdapter();
        adap.clear();
        adap.notifyDataSetChanged();

        String strURL = "http://dprc.gov.ua/awg/xml?class_name=IStations&method_name=search_station&var_0=2&var_1=2&var_2=0&var_3=16&var_4=" +
                spinnerA.getItemAtPosition(pos);

        String strResponse = null;
        try {
            strResponse = sendHTTPRequest(strURL);
        }catch (IOException e){
            messageBox("Увага", "Exception in fillStations(): " + e.getMessage());
            return false;
        }

        if (strResponse == "")
            return false;
        messageBox("Увага", "Response:  " +  strResponse);
        return true;
    }

    private String sendHTTPRequest(String strURL) throws IOException {
        String strResponse = "";
        URL url = null;
        BufferedReader reader=null;
        HttpURLConnection connect = null;
        try {
            url = new URL(strURL);
            connect = (HttpURLConnection)url.openConnection();
            connect.setReadTimeout(10000 /* milliseconds */);
            connect.setConnectTimeout(10000 /* milliseconds */);
            connect.setRequestMethod("GET");
            connect.setDoInput(true);
            connect.connect();
            int nHTTPResonse = connect.getResponseCode();
            if (nHTTPResonse == HttpURLConnection.HTTP_OK) {
                reader = new BufferedReader(new InputStreamReader(connect.getInputStream()));
                StringBuilder strBuf = new StringBuilder();
                while ((strResponse =reader.readLine()) != null) {
                    strBuf.append(strResponse + "\n");
                }
            } else {
                // ошибка
                messageBox("Увага", strURL + " повернув помилку: " + nHTTPResonse);
            }

        } catch (Exception e) {
            messageBox("Увага", "Exception in sendHTTPReques(): " + e.getMessage());
        }
        finally {
            if (reader != null)
                reader.close();
            if (connect != null)
                connect.disconnect();
        }
        return strResponse;
    }


}