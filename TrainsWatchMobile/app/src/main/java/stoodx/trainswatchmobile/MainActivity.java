package stoodx.trainswatchmobile;

import android.app.AlertDialog;
import android.app.VoiceInteractor;
import android.content.DialogInterface;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;

import java.util.ArrayList;
import java.util.List;


public class MainActivity extends AppCompatActivity implements AdapterView.OnItemSelectedListener {

    public class Station {
        public String m_strID;
        public String m_strName;
    }

    private List<Station> m_arrayStationsFrom;
    private List<Station> m_arrayStationsTo;

    private Spinner m_spinnerFromA;
    private Spinner m_spinnerToA;
    private Spinner m_spinnerFrom;
    private Spinner m_spinnerTo;

    private int m_nIDSpinner;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        m_arrayStationsFrom = new ArrayList<Station>();
        m_arrayStationsTo = new ArrayList<Station>();

        //From
        m_spinnerFromA = (Spinner) findViewById(R.id.spinnerFromA);
        m_spinnerFromA.setOnItemSelectedListener(this);
        ArrayAdapter<CharSequence> adapterFromA = ArrayAdapter.createFromResource(this,
                R.array.spinnerFromA, android.R.layout.simple_spinner_item);
        adapterFromA.setDropDownViewResource(android.R.layout.simple_spinner_item);
        m_spinnerFromA.setAdapter(adapterFromA);

        m_spinnerFrom = (Spinner) findViewById(R.id.spinnerFrom);
        List<String> listFrom = new ArrayList<String>();
        ArrayAdapter<String> adapterFrom = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item,  listFrom);
        adapterFrom.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        m_spinnerFrom.setAdapter(adapterFrom);

        //To
        m_spinnerToA = (Spinner) findViewById(R.id.spinnerToA);
        m_spinnerToA.setOnItemSelectedListener(this);
        ArrayAdapter<CharSequence> adapterToA = ArrayAdapter.createFromResource(this,
                R.array.spinnerToA, android.R.layout.simple_spinner_item);
        adapterToA.setDropDownViewResource(android.R.layout.simple_spinner_item);
        m_spinnerToA.setAdapter(adapterToA);

        m_spinnerTo = (Spinner) findViewById(R.id.spinnerTo);
        List<String> listTo = new ArrayList<String>();
        ArrayAdapter<String> adapterTo = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item,  listTo);
        adapterFrom.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        m_spinnerTo.setAdapter(adapterTo);
    }

    public void onItemSelected(AdapterView<?> parent, View view,
                               int pos, long id) {
        // An item was selected. You can retrieve the selected item using
        // parent.getItemAtPosition(pos)

        int iID = parent.getId();
        switch (iID) {
            case R.id.spinnerFromA:
                sendStationsFilling(m_spinnerFromA, pos);
                break;
            case R.id.spinnerToA:
                sendStationsFilling(m_spinnerToA, pos);
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

    private boolean sendStationsFilling(Spinner spinnerA,  int pos) {
        //clear all lists
        String strURL = "http://dprc.gov.ua/awg/xml?class_name=IStations&method_name=search_station&var_0=3&var_1=2&var_2=0&var_3=16&var_4=" +
                spinnerA.getItemAtPosition(pos);

        sendHTTPRequest(strURL, spinnerA.getId());
        return true;
    }

    private void sendHTTPRequest(String strURL, int id) {
        // Instantiate the RequestQueue.
        RequestQueue queue = Volley.newRequestQueue(this);
        // Request a string response from the provided URL.
        m_nIDSpinner = id;
        StringRequest stringRequest = new StringRequest(Request.Method.GET, strURL,
                new Response.Listener<String>() {
                    @Override
                    public void onResponse(String strResponse) {
                        handleResponse(strResponse, m_nIDSpinner);
                     }
                }, new Response.ErrorListener() {
            @Override
            public void onErrorResponse(VolleyError error) {
                messageBox("Увага", "Помилка з сайту: " +  error.getMessage());
            }
        });
        // Add the request to the RequestQueue.
        queue.add(stringRequest);

    }

    private void handleResponse(String strResponse, int id){
            switch (id) {
            case R.id.spinnerFromA:
                fillStations(strResponse, m_spinnerFrom, m_arrayStationsFrom);
                break;
            case R.id.spinnerToA:
                fillStations(strResponse, m_spinnerTo, m_arrayStationsTo);
                break;
            default:
                break;
        }
    }

    private boolean fillStations(String strResponse, Spinner spinner, List<Station> arr)
    {
        arr.clear();
        ArrayAdapter adap = (ArrayAdapter) spinner.getAdapter();
        adap.clear();
        adap.notifyDataSetChanged();

        int nIndex = strResponse.indexOf("<MSG><var_0>");
        if (nIndex == -1){
            messageBox("Увага", "Зіпсований формат з сайту");
            return false;
        }
        nIndex += "<MSG><var_0>".length();
        strResponse =  strResponse.substring(nIndex);
        while(true){
            nIndex = strResponse.indexOf("<childs><i v=\"");
            if (nIndex == -1)
                break;
            nIndex += "<childs><i v=\"".length();
            strResponse =  strResponse.substring(nIndex);
            int nLen = strResponse.length();
            if ( nLen == 0)
                break;
           //id;
            int i;
            char c;
            String strID = "";
            for(i = 0; i < nLen; i++){
                c = strResponse.charAt(i);
                if (c == '\"')
                    break;
                strID += c;
            }
            //name
            nIndex = strResponse.indexOf("<i v=\"");
            if (nIndex == -1)
                break;
            nIndex += "<i v=\"".length();
            strResponse = strResponse.substring(nIndex);
            nLen = strResponse.length();
            if (nLen == 0)
                break;
            String strName = "";
            for (i = 0; i < nLen; i++){
                c = strResponse.charAt(i);
                if (c == '\"')
                    break;
                strName += c;
            }
            Station station = new Station();
            station.m_strID = strID;
            station.m_strName = strToUTF16(strName);
            arr.add(station);
        }

        int nSize = arr.size();
        if (nSize == 0){
            messageBox("Увага", "Немає станцій");
            return false;
        }
        for(int j = 0; j < nSize; j++){
            Station st = arr.get(j);
            adap.add(st.m_strName);
        }
        adap.notifyDataSetChanged();
        spinner.setSelection(0);
        return true;
    }

    private String strToUTF16(String str){

        String strUTF6 = str;
        try {
            strUTF6 = new String(str.getBytes("UTF-16"), "windows-1251");
        } catch (Exception e){
            messageBox("Увага", "Exception: " +  e.getMessage());
        }
        return strUTF6.substring(2);
    }
}