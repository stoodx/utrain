package stoodx.trainswatchmobile;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import com.android.volley.Cache;
import com.android.volley.Network;
import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.BasicNetwork;
import com.android.volley.toolbox.DiskBasedCache;
import com.android.volley.toolbox.HurlStack;
import com.android.volley.toolbox.JsonObjectRequest;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;

import com.loopj.android.http.*;

import java.io.IOException;
import java.io.StringReader;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import cz.msebera.android.httpclient.Header;

public class MainActivity extends AppCompatActivity implements AdapterView.OnItemSelectedListener {

    public class Station {
        public String m_strID;
        public String m_strName;
    }

    public class ParserParameter{
        public String str1;
        public String str2;
        public String str3;
    }

    private List<Station> m_arrayStationsFrom;
    private List<Station> m_arrayStationsTo;

    private Spinner m_spinnerFromA;
    private Spinner m_spinnerToA;
    private Spinner m_spinnerFrom;
    private Spinner m_spinnerTo;

    private int[] m_nIDSpinner;

    private String m_strCalendar;

    private  String m_strToken;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        m_nIDSpinner = new int[4];
        m_nIDSpinner[0] = 0;
        m_nIDSpinner[1] = 0;
        m_nIDSpinner[2] = 0;
        m_nIDSpinner[3] = 0;

        m_arrayStationsFrom = new ArrayList<Station>();
        m_arrayStationsTo = new ArrayList<Station>();

        m_strCalendar = "";
        m_strToken = "";

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

        SendRequestForToken();
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
    //    String strURL = "http://dprc.gov.ua/awg/xml?class_name=IStations&method_name=search_station&var_0=3&var_1=2&var_2=0&var_3=16&var_4=" +
    //            spinnerA.getItemAtPosition(pos);
        String strURL = "http://booking.uz.gov.ua/purchase/station/" +
                spinnerA.getItemAtPosition(pos);
        sendHTTPRequest(strURL, spinnerA.getId());
        return true;
    }

    private void SendRequestForToken(){
        sendHTTPRequest("http://booking.uz.gov.ua", -2);
    }


    private void sendHTTPRequest(String strURL, int id){
        int i;
        synchronized (m_nIDSpinner) {
            for (i = 0; i < 4; i++) {
                if (m_nIDSpinner[i] == 0) {
                    m_nIDSpinner[i] = id;
                    break;
                }
            }
        }
        if (id == 4){
            messageBox("Увага", "Перевищено поріг запитів до сайту");
            return;
        }
        AsyncHttpClient client = new AsyncHttpClient();
        client.setMaxRetriesAndTimeout(10, 10000);
        client.get(strURL, new AsyncHttpResponseHandler() {
            @Override
            public void onSuccess(int statusCode, Header[] headers, byte[] response) {
                // called when response HTTP status is "200 OK"
                try {
                    String strResponse = String.valueOf(new String(response, "UTF-8"));
                    handleResponse(strResponse, headers);
                } catch (UnsupportedEncodingException e) {
                    messageBox("Увага", "Exception: " + e.getMessage());
                }
            }

            @Override
            public void onFailure(int statusCode, Header[] headers, byte[] errorResponse, Throwable e) {
                // called when response HTTP status is "4XX" (eg. 401, 403, 404)
                messageBox("Увага", "Помилка з сайту: " + errorResponse.toString());
            }
        });
    }

/*
    private void sendHTTPRequest(String strURL, int id) {
        RequestQueue queue = Volley.newRequestQueue(this);
        // Request a string response from the provided URL.
        int i;
        synchronized (m_nIDSpinner) {
            for (i = 0; i < 4; i++) {
                if (m_nIDSpinner[i] == 0) {
                    m_nIDSpinner[i] = id;
                    break;
                }
            }
        }
        if (id == 4){
            messageBox("Увага", "Перевищено поріг запитів до сайту");
            return;
        }
        StringRequest stringRequest = new StringRequest(Request.Method.GET,  strURL,
               new Response.Listener<String>() {
                    @Override
                    public void onResponse(String strResponse) {
                        handleResponse(strResponse);
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
*/
    private void handleResponse(String strResponse, Header[] headers){
        int id = 0;
        int i;
        synchronized (m_nIDSpinner) {
            for (i = 0; i < 4; i++) {
                if (m_nIDSpinner[i] != 0) {
                    id = m_nIDSpinner[i];
                    m_nIDSpinner[i] = 0;
                    break;
                }
            }
        }
        if (i == 4){
            return;
        }
        switch (id) {
            case R.id.spinnerFromA:
                fillStationsBooking(strResponse, m_spinnerFrom, m_arrayStationsFrom);
                break;
            case R.id.spinnerToA:
                fillStationsBooking(strResponse, m_spinnerTo, m_arrayStationsTo);
                break;
            case -1: //request
                responseRequest(strResponse);
                break;
            case -2:
                responseToken(strResponse, headers);
                break;
            default:
                break;
        }
    }

    private boolean responseToken(String strResponse, Header[] headers){

        if (headers.length == 0){
            messageBox("Увага", "Нема cookies з сайту: " + strResponse);
            return false;
        }

        int nIndex = strResponse.indexOf("gaq.push(['_trackPageview']);");
        if (nIndex == -1){
            messageBox("Увага", "Зіпсований формат токена з сайту: " + strResponse);
            return false;
        }
        nIndex += "gaq.push(['_trackPageview']);".length();
        strResponse =  strResponse.substring(nIndex);
        nIndex = strResponse.indexOf("(function ()");
        if (nIndex == -1){
            messageBox("Увага", "Зіпсований формат токена з сайту: " + strResponse);
            return false;
        }
        String strTokenEncode =  strResponse.substring(0, nIndex);
        m_strToken = jjdecode(strTokenEncode);

        return true;
    }

    private String jjdecode(String it){
        String buffer = "";
        int j, k;

        //clean it
        String t = it.replace("/^\\s+|\\s+$/g", "");

        int startpos;
        int endpos;
        String gv;
        String  gvl;

        if (t.indexOf("\"\'\\\"+\'+\",") == 0) //palindrome check
        {
            //locate jjcode
            startpos	= t.indexOf('$$+"\\""+') + 8;
            endpos		= t.indexOf('"\\"")())()');

            //get gv
            gv	= t.substring((t.indexOf('"\'\\"+\'+",')+9), t.indexOf("=~[]"));
            gvl	= gv.length;
        }
        else
        {
            //get gv
            gv	= t.substr(0, t.indexOf("="));
            gvl	= gv.length;

            //locate jjcode
            startpos	= t.indexOf('"\\""+') + 5;
            endpos		= t.indexOf('"\\"")())()');
        }

        if (startpos == endpos)
        {
            alert("No data !");
            return;
        }

        //start decoding
        var data = t.substring(startpos, endpos);

        //hex decode string
        var b=[ "___+", "__$+", "_$_+", "_$$+", "$__+", "$_$+", "$$_+", "$$$+", "$___+", "$__$+", "$_$_+", "$_$$+", "$$__+", "$$_$+", "$$$_+", "$$$$+" ];

        //lotu
        var str_l = "(![]+\"\")[" + gv + "._$_]+";
        var str_o = gv + "._$+";
        var str_t = gv + ".__+";
        var str_u = gv + "._+";

        //0123456789abcdef
        var str_hex = gv + ".";

        //s
        var str_s = '"';
        var gvsig = gv + ".";

        var str_quote = '\\\\\\"';
        var str_slash = '\\\\\\\\';

        var str_lower = "\\\\\"+";
        var str_upper = "\\\\\"+" + gv + "._+";

        var str_end	= '"+'; //end of s loop


        while(data != "")
        {
            //l o t u
            if (0 == data.indexOf(str_l))
            {
                data = data.substr(str_l.length);
                buffer += "l";
                continue;
            }
            else if (0 == data.indexOf(str_o))
            {
                data = data.substr(str_o.length);
                buffer += "o";
                continue;
            }
            else if (0 == data.indexOf(str_t))
            {
                data = data.substr(str_t.length);
                buffer += "t";
                continue;
            }
            else if (0 == data.indexOf(str_u))
            {
                data = data.substr(str_u.length);
                buffer += "u";
                continue;
            }

            //0123456789abcdef
            if (0 == data.indexOf(str_hex))
            {
                data = data.substr(str_hex.length);

                //check every element of hex decode string for a match
                var i = 0;
                for (i = 0; i < b.length; i++)
                {
                    if (0 == data.indexOf(b[i]))
                    {
                        data = data.substr( (b[i]).length );
                        buffer += i.toString(16);
                        break;
                    }
                }
                continue;
            }

            //start of s block
            if (0 == data.indexOf(str_s))
            {
                data = data.substr(str_s.length);

                //check if "R
                if (0 == data.indexOf(str_upper)) // r4 n >= 128
                {
                    data = data.substr(str_upper.length); //skip sig

                    var ch_str = "";
                    for (j = 0; j < 2; j++) //shouldn't be more than 2 hex chars
                    {
                        //gv + "."+b[ c ]
                        if (0 == data.indexOf(gvsig))
                        {
                            data = data.substr(gvsig.length); //skip gvsig

                            for (k = 0; k < b.length; k++)	//for every entry in b
                            {
                                if (0 == data.indexOf(b[k]))
                                {
                                    data = data.substr(b[k].length);
                                    ch_str += k.toString(16) + "";
                                    break;
                                }
                            }
                        }
                        else
                        {
                            break; //done
                        }
                    }

                    buffer += String.fromCharCode(parseInt(ch_str,16));
                    continue;
                }
                else if (0 == data.indexOf(str_lower)) //r3 check if "R // n < 128
                {
                    data = data.substr(str_lower.length); //skip sig

                    var ch_str = "";
                    var ch_lotux = ""
                    var temp = "";
                    var b_checkR1 = 0;
                    for (j = 0; j < 3; j++) //shouldn't be more than 3 octal chars
                    {

                        if (j > 1) //lotu check
                        {
                            if (0 == data.indexOf(str_l))
                            {
                                data = data.substr(str_l.length);
                                ch_lotux = "l";
                                break;
                            }
                            else if (0 == data.indexOf(str_o))
                            {
                                data = data.substr(str_o.length);
                                ch_lotux = "o";
                                break;
                            }
                            else if (0 == data.indexOf(str_t))
                            {
                                data = data.substr(str_t.length);
                                ch_lotux = "t";
                                break;
                            }
                            else if (0 == data.indexOf(str_u))
                            {
                                data = data.substr(str_u.length);
                                ch_lotux = "u";
                                break;
                            }
                        }

                        //gv + "."+b[ c ]
                        if (0 == data.indexOf(gvsig))
                        {
                            temp = data.substr(gvsig.length);
                            for (k = 0; k < 8; k++)	//for every entry in b octal
                            {
                                if (0 == temp.indexOf(b[k]))
                                {
                                    if (parseInt(ch_str + k + "",8) > 128)
                                    {
                                        b_checkR1 = 1;
                                        break;
                                    }

                                    ch_str += k + "";
                                    data = data.substr(gvsig.length); //skip gvsig
                                    data = data.substr(b[k].length);
                                    break;
                                }
                            }

                            if (1 == b_checkR1)
                            {
                                if (0 == data.indexOf(str_hex)) //0123456789abcdef
                                {
                                    data = data.substr(str_hex.length);

                                    //check every element of hex decode string for a match
                                    var i = 0;
                                    for (i = 0; i < b.length; i++)
                                    {
                                        if (0 == data.indexOf(b[i]))
                                        {
                                            data = data.substr( (b[i]).length );
                                            ch_lotux = i.toString(16);
                                            break;
                                        }
                                    }

                                    break;
                                }
                            }
                        }
                        else
                        {
                            break; //done
                        }
                    }

                    buffer += String.fromCharCode(parseInt(ch_str,8)) + ch_lotux;
                    continue; //step out of the while loop
                }
                else //"S ----> "SR or "S+
                {

                    // if there is, loop s until R 0r +
                    // if there is no matching s block, throw error

                    var match = 0;
                    var n;

                    //searching for mathcing pure s block
                    while(true)
                    {
                        n = data.charCodeAt( 0 );
                        if (0 == data.indexOf(str_quote))
                        {
                            data = data.substr(str_quote.length);
                            buffer += '"';
                            match += 1;
                            continue;
                        }
                        else if (0 == data.indexOf(str_slash))
                        {
                            data = data.substr(str_slash.length);
                            buffer += '\\';
                            match += 1;
                            continue;
                        }
                        else if (0 == data.indexOf(str_end))	//reached end off S block ? +
                        {
                            if (match == 0)
                            {
                                alert("+ no match S block: "+data);
                                return;
                            }
                            data = data.substr(str_end.length);

                            break; //step out of the while loop
                        }
                        else if (0 == data.indexOf(str_upper)) //r4 reached end off S block ? - check if "R n >= 128
                        {
                            if (match == 0)
                            {
                                alert("no match S block n>128: "+data);
                                return;
                            }

                            data = data.substr(str_upper.length); //skip sig

                            var ch_str = "";
                            var ch_lotux = "";
                            for (j = 0; j < 10; j++) //shouldn't be more than 10 hex chars
                            {

                                if (j > 1) //lotu check
                                {
                                    if (0 == data.indexOf(str_l))
                                    {
                                        data = data.substr(str_l.length);
                                        ch_lotux = "l";
                                        break;
                                    }
                                    else if (0 == data.indexOf(str_o))
                                    {
                                        data = data.substr(str_o.length);
                                        ch_lotux = "o";
                                        break;
                                    }
                                    else if (0 == data.indexOf(str_t))
                                    {
                                        data = data.substr(str_t.length);
                                        ch_lotux = "t";
                                        break;
                                    }
                                    else if (0 == data.indexOf(str_u))
                                    {
                                        data = data.substr(str_u.length);
                                        ch_lotux = "u";
                                        break;
                                    }
                                }

                                //gv + "."+b[ c ]
                                if (0 == data.indexOf(gvsig))
                                {
                                    data = data.substr(gvsig.length); //skip gvsig

                                    for (k = 0; k < b.length; k++)	//for every entry in b
                                    {
                                        if (0 == data.indexOf(b[k]))
                                        {
                                            data = data.substr(b[k].length);
                                            ch_str += k.toString(16) + "";
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    break; //done
                                }
                            }

                            buffer += String.fromCharCode(parseInt(ch_str,16));
                            break; //step out of the while loop
                        }
                        else if (0 == data.indexOf(str_lower)) //r3 check if "R // n < 128
                        {
                            if (match == 0)
                            {
                                alert("no match S block n<128: "+data);
                                return;
                            }

                            data = data.substr(str_lower.length); //skip sig

                            var ch_str = "";
                            var ch_lotux = ""
                            var temp = "";
                            var b_checkR1 = 0;
                            for (j = 0; j < 3; j++) //shouldn't be more than 3 octal chars
                            {

                                if (j > 1) //lotu check
                                {
                                    if (0 == data.indexOf(str_l))
                                    {
                                        data = data.substr(str_l.length);
                                        ch_lotux = "l";
                                        break;
                                    }
                                    else if (0 == data.indexOf(str_o))
                                    {
                                        data = data.substr(str_o.length);
                                        ch_lotux = "o";
                                        break;
                                    }
                                    else if (0 == data.indexOf(str_t))
                                    {
                                        data = data.substr(str_t.length);
                                        ch_lotux = "t";
                                        break;
                                    }
                                    else if (0 == data.indexOf(str_u))
                                    {
                                        data = data.substr(str_u.length);
                                        ch_lotux = "u";
                                        break;
                                    }
                                }

                                //gv + "."+b[ c ]
                                if (0 == data.indexOf(gvsig))
                                {
                                    temp = data.substr(gvsig.length);
                                    for (k = 0; k < 8; k++)	//for every entry in b octal
                                    {
                                        if (0 == temp.indexOf(b[k]))
                                        {
                                            if (parseInt(ch_str + k + "",8) > 128)
                                            {
                                                b_checkR1 = 1;
                                                break;
                                            }

                                            ch_str += k + "";
                                            data = data.substr(gvsig.length); //skip gvsig
                                            data = data.substr(b[k].length);
                                            break;
                                        }
                                    }

                                    if (1 == b_checkR1)
                                    {
                                        if (0 == data.indexOf(str_hex)) //0123456789abcdef
                                        {
                                            data = data.substr(str_hex.length);

                                            //check every element of hex decode string for a match
                                            var i = 0;
                                            for (i = 0; i < b.length; i++)
                                            {
                                                if (0 == data.indexOf(b[i]))
                                                {
                                                    data = data.substr( (b[i]).length );
                                                    ch_lotux = i.toString(16);
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    break; //done
                                }
                            }

                            buffer += String.fromCharCode(parseInt(ch_str,8)) + ch_lotux;
                            break; //step out of the while loop
                        }
                        else if( (0x21 <= n && n <= 0x2f) || (0x3A <= n && n <= 0x40) || ( 0x5b <= n && n <= 0x60 ) || ( 0x7b <= n && n <= 0x7f ) )
                        {
                            buffer += data.charAt( 0 );
                            data = data.substr( 1 );
                            match += 1;
                        }

                    }
                    continue;
                }
            }

            alert("no match : "+data);
            break;
        }

        return buffer.toString();


        return buffer;
    }

    private boolean fillStationsBooking(String strResponse, Spinner spinner, List<Station> arr)
    {
        arr.clear();
        ArrayAdapter adap = (ArrayAdapter) spinner.getAdapter();
        adap.clear();
        adap.notifyDataSetChanged();

        int nIndex = strResponse.indexOf("{\"value\":[{");
        if (nIndex == -1){
            messageBox("Увага", "Зіпсований формат з сайту: " + strResponse);
            return false;
        }
        nIndex += "{\"value\":[{".length();
        strResponse =  strResponse.substring(nIndex);
        while(true){
            nIndex = strResponse.indexOf("\"title\":\"");
            if (nIndex == -1)
                break;

            nIndex += "\"title\":\"".length();
            strResponse =  strResponse.substring(nIndex);
            int nLen = strResponse.length();
            if ( nLen == 0)
                break;

            //station name
            int i;
            char c;
            String strName = "";
            for (i = 0; i < nLen; i++){
                c = strResponse.charAt(i);
                if (c == '\"')
                    break;
                strName += c;
            }

            //id;
            nIndex = strResponse.indexOf("\"station_id\":");
            if (nIndex == -1)
                break;
            nIndex += "\"station_id\":".length();
            strResponse =  strResponse.substring(nIndex);
            nLen = strResponse.length();
            if ( nLen == 0)
                break;
            String strID = "";
            for(i = 0; i < nLen; i++){
                c = strResponse.charAt(i);
                if (c == '}')
                    break;
                strID += c;
            }

            Station station = new Station();
            station.m_strID = strID;
            station.m_strName = printUTF8Converter(strName);
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

    private String printUTF8Converter(String str){
        String strResponse = "";
        if (str.length()== 0)
            return strResponse;
        try {
            Properties p = new Properties();
            p.load(new StringReader("key=" + str));
            strResponse = p.getProperty("key");
        }catch (IOException e){
            messageBox("Увага", "Exception: " +  e.getMessage());
        }

        return strResponse;
    }


    private boolean fillStationsDPRC(String strResponse, Spinner spinner, List<Station> arr)
    {
        arr.clear();
        ArrayAdapter adap = (ArrayAdapter) spinner.getAdapter();
        adap.clear();
        adap.notifyDataSetChanged();

        int nIndex = strResponse.indexOf("<MSG><var_0>");
        if (nIndex == -1){
            messageBox("Увага", "Зіпсований формат з сайту: " + strResponse);
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

    static final private int CHOOSE_CALENDAR = 0;

    public void onClickWhen(View view) {
        Intent intent = new Intent(MainActivity.this, CalendarActivity.class);
        startActivityForResult(intent, CHOOSE_CALENDAR);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data){
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == CHOOSE_CALENDAR){
            if (resultCode == RESULT_OK){
                m_strCalendar = data.getStringExtra(CalendarActivity.CALENDAR);
            }
        }
    }

    public void onClickRequest(View view) {
        if (m_strCalendar.length() == 0){
            messageBox("Увага", "Вам необхідно вибрати дату відправки. Для цього натисніть на кнопку Коли.");
            return;
        }

        int nPosFrom = m_spinnerFrom.getSelectedItemPosition();
        int nPosTo = m_spinnerTo.getSelectedItemPosition();

        Station stationFrom = m_arrayStationsFrom.get(nPosFrom);
        Station stationTo = m_arrayStationsTo.get(nPosTo);


        String strURL = "http://dprc.gov.ua/show.php?transport_type=2&src=" +
               stationFrom.m_strID +
                "&dst=" +
                stationTo.m_strID +
                "&dt=" +
                m_strCalendar +
                "&ret_dt=2001-01-01&ps=ec_privat&set_language=1";

        sendHTTPRequest(strURL, -1);
    }

    private void responseRequest(String strResponse){
         messageBox("Iнформація", parser(strResponse));
    }

    final private static String CR = "\n";

    private  String parser(String strResponse){
        String  strJSONResult = "";
        if (strResponse.length() == 0){
            strJSONResult =  "{\"error\":\"No data for operation\"}";
            return strJSONResult;
        }

        String strFrom = "";
        String strTo = "";
        String strDate = "";
        int nIndex = 0;
        int nLen = 0;
        int i = 0;
        char ch = 0;
        List<String> listTrains = new ArrayList<String>();;

        //target
        nIndex = strResponse.indexOf("<div id=\"tables\" class='tables'>");
        if (nIndex == -1)
        {
            strJSONResult ="{\"error\":\"No information\"}";
            return strJSONResult;
        }
        nIndex += "<div id=\"tables\" class='tables'>".length();
        strResponse = strResponse.substring(nIndex);
        if (strResponse.length() == 0)
        {
            strJSONResult ="{\"error\":\"No table\"}";
            return strJSONResult;
        }

        //date
        nIndex = strResponse.indexOf("<span style='font-weight: bold;'>");
        if (nIndex == -1)
        {
            strJSONResult ="{\"error\":\"No date1\"}";
            return strJSONResult;
        }
        nIndex += "<span style='font-weight: bold;'>".length();
        strResponse = strResponse.substring(nIndex);
        nLen = strResponse.length();
        if (nLen == 0)
        {
            strJSONResult = "{\"error\":\"No date2\"}";
            return strJSONResult;
        }
        for (i = 0; i < nLen; i++)
        {
            ch = strResponse.charAt(i);
            if (ch == '<')
                break;
            strDate += ch;
        }
        if (strDate.length() == 0)
        {
            strJSONResult ="{\"error\":\"No date3\"}";
            return strJSONResult;
        }
        //from
        nIndex = strResponse.indexOf("<span style='font-weight: bold;'>");
        if (nIndex == -1)
        {
            strJSONResult = "{\"error\":\"No data for departure1\"}";
            return strJSONResult;
        }
        nIndex +=  "<span style='font-weight: bold;'>".length();
        strResponse = strResponse.substring(nIndex);
        nLen = strResponse.length();
        if (nLen == 0)
        {
            strJSONResult = "{\"error\":\"No data for departure2\"}";
            return strJSONResult;
        }
        for (i = 0; i < nLen; i++)
        {
            ch = strResponse.charAt(i);
            if (ch == '<')
                break;
            strFrom += ch;
        }
        if (strFrom.length() == 0)
        {
            strJSONResult = "{\"error\":\"No data for departure3\"}";
            return strJSONResult;
        }
        //to
        nIndex = strResponse.indexOf("<span style='font-weight: bold;'>");
        if (nIndex == -1)
        {
            strJSONResult = "{\"error\":\"No data for destination1\"}";
            return strJSONResult;
        }
        nIndex +=  "<span style='font-weight: bold;'>".length();
        strResponse = strResponse.substring(nIndex);
        nLen = strResponse.length();
        if (nLen == 0)
        {
            strJSONResult = "{\"error\":\"No data for destination2\"}";
            return strJSONResult;
        }
        for (i = 0; i < nLen; i++)
        {
            ch = strResponse.charAt(i);
            if (ch == '<')
            break;
            strTo += ch;
        }
        if (strTo.length() == 0)
        {
            strJSONResult = "{\"error\":\"No data for destination3\"}";
            return strJSONResult;
        }

        //Trains;
        while(true){
            String strTrainNumber = "";
            String strTrainDeparture = "";
            String strTrainDestination = "";
            String strTrainDep = "";
            String strTrainDuration = "";
            String strTrainArrive = "";
            String strTrainLuxPrice = "";
            String strTrainLuxSeat = "";
            String strTrainCompartmentFirmPrice = "";
            String strTrainCompartmentFirmSeat = "";
            String strTrainCompartmentPrice = "";
            String strTrainCompartmentSeat = "";
            String strTrainThirdClassFirmPrice = "";
            String strTrainThirdClassFirmSeat = "";
            String strTrainThirdClassPrice = "";
            String strTrainThirdClassSeat = "";
            String strTrainSeatsPrice = "";
            String strTrainSeatsSeat = "";

            ParserParameter par = new ParserParameter();

            nIndex = strResponse.indexOf("<tr class=\"train_row\" id=\"row_");
            if (nIndex == -1)
                break;
            nIndex +=  "<tr class=\"train_row\" id=\"row_".length();
            strResponse = strResponse.substring(nIndex);
            if (strResponse.length() == 0)
                break;
            //number
            par.str1 = strResponse;
            if (!partParser(par,
                    "<td class=\"info_row train first\" style='font-size: 14pt; vertical-align: top; margin-top: 0px; padding-top: 1px; padding-right: 0px;'>"))
                break;
            strResponse = par.str1;
            strTrainNumber = par.str2;
            if (strTrainNumber.length() == 0)
                break;

            //departue
            par.str1 = strResponse;
            if (!partParser(par, "<td class=\"info_row name\">"))
                break;
            strResponse = par.str1;
            strTrainDeparture = par.str2;
            if (strTrainDeparture.length() == 0)
                break;

            //dep.
            par.str1 = strResponse;
            if (!partParser(par, "<td class=\"info_row depart\">"))
                break;
            strResponse = par.str1;
            strTrainDep = par.str2;
            if (strTrainDep.length() == 0)
                break;

            //duration
            par.str1 = strResponse;
            if (!partParser(par, "<td class=\"info_row onway\">&nbsp;"))
                break;
            strResponse = par.str1;
            strTrainDuration = par.str2;
            if (strTrainDuration.length() == 0)
                break;

            //arrive
            par.str1 = strResponse;
            if (!partParser(par, "<td class=\"info_row arrive\">"))
                break;
            strResponse = par.str1;
            strTrainArrive = par.str2;
            if (strTrainArrive.length() == 0)
                break;

            //lux
            par.str1 = strResponse;
            partParserWagon(par, " c_1050\">");
            strResponse = par.str1;
            strTrainLuxPrice = par.str2;
            strTrainLuxSeat = par.str3;

            //compartment firm
            par.str1 = strResponse;
            partParserWagon(par, " c_1040\">");
            strResponse = par.str1;
            strTrainCompartmentFirmPrice = par.str2;
            strTrainCompartmentFirmSeat = par.str3;

            //compartment
            par.str1 = strResponse;
            partParserWagon(par, " c_1030\">");
            strResponse = par.str1;
            strTrainCompartmentPrice = par.str2;
            strTrainCompartmentSeat = par.str3;

            //third class firm
            par.str1 = strResponse;
            partParserWagon(par, " c_1025\">");
            strResponse = par.str1;
            strTrainThirdClassFirmPrice = par.str2;
            strTrainThirdClassFirmSeat = par.str3;

            //third class
            par.str1 = strResponse;
            partParserWagon(par, " c_1020\">");
            strResponse = par.str1;
            strTrainThirdClassPrice = par.str2;
            strTrainThirdClassSeat = par.str3;

            //left seats
            par.str1 = strResponse;
            partParserWagon(par, " c_1001 last\">");
            strResponse = par.str1;
            strTrainSeatsPrice = par.str2;
            strTrainSeatsSeat = par.str3;

            String strSumTrains = String.format(
                    "\"train\": {%s\"number\": \"%s\", \"departure\": \"%s\", \"destination\": \"%s\", \"dep.\": \"%s\", \"duration\": \"%s\", \"arrive\": \"%s\", %s\"lux\": {\"price\": \"%s\", \"seats\": \"%s\"}, \"compartment_firm\": {\"price\": \"%s\", \"seats\": \"%s\"}, \"compartment\": {\"price\": \"%s\", \"seats\": \"%s\"}, %s\"third_class_firm\": {\"price\": \"%s\", \"seats\": \"%s\"},  \"third_class\": {\"price\": \"%s\", \"seats\": \"%s\"}, \"left_seats\": {\"price\": \"%s\", \"seats\": \"%s\"} },%s",
                    CR, strTrainNumber, strTrainDeparture, strTrainDestination, strTrainDep, strTrainDuration, strTrainArrive,
                    CR, strTrainLuxPrice, strTrainLuxSeat,
                    strTrainCompartmentFirmPrice, strTrainCompartmentFirmSeat,
                    strTrainCompartmentPrice, strTrainCompartmentSeat,
                    CR, strTrainThirdClassFirmPrice, strTrainThirdClassFirmSeat,
                    strTrainThirdClassPrice, strTrainThirdClassSeat,
                    strTrainSeatsPrice,  strTrainSeatsSeat,
                    CR);
            listTrains.add(strSumTrains);
        }//end white

        strJSONResult = String.format(
                "{\"target\":{%s\"date\": \"%s\",%s\"from\": \"%s\",%s\"to\": \"%s\"%s},%s\"trains\":{%s",
                CR, strDate,CR,strFrom,CR,strTo,CR,CR,CR, CR
        );
        int nSize = listTrains.size();
        for(i = 0; i < nSize; i++){
            strJSONResult += listTrains.get(i);
        }
        strJSONResult += "},";
        strJSONResult += CR;
        strJSONResult += "}";
        return strJSONResult;
    }

    private boolean partParser(ParserParameter par, String str){
        String strResponse = par.str1;
        String strTarget = "";

        if (strResponse.length() == 0 || str.length() == 0)
            return false;
        int i, nIndex;

        strTarget = "";
        nIndex = strResponse.indexOf(str);
        if (nIndex == -1)
            return false;
        nIndex +=  str.length();
        strResponse = strResponse.substring(nIndex);
        int nLen = strResponse.length();
        if (nLen == 0)
            return false;

        for (i = 0; i < nLen; i++)
        {
            char ch = strResponse.charAt(i);
            if (ch == '<')
                break;
            strTarget += ch;
        }
        par.str1 = strResponse;
        par.str2 = strTarget;
        return true;
    }

    private boolean partParserWagon(ParserParameter par, String str){
        String strResponse = par.str1;
        String strPrice = "";
        String strSeats = "";

        if (strResponse.length() == 0 || str.length() == 0)
            return false;

        int i, nIndex1, nIndex2;

        boolean bResult = false;
        String strSub = "";

        nIndex1 = strResponse.indexOf(str);
        if (nIndex1 == -1)
            return bResult;
        nIndex1 += str.length();
        strResponse = strResponse.substring(nIndex1);
        if (strResponse.length() == 0)
            return bResult;
        nIndex2 = strResponse.indexOf("</td>");
        if (nIndex2 == -1)
        {
            return bResult;
        }
        if (nIndex2 == 0)
        {//no info
            //goto end;
            nIndex2 +=  "</td>".length();
            strResponse = strResponse.substring(nIndex2);
            par.str1 = strResponse;
            par.str2 = strPrice;
            par.str3 = strSeats;
            return bResult;
        }

        strSub = strResponse.substring(0, nIndex2 + 1);
        if (strSub.length() == 0){
            //goto end;
            nIndex2 +=  "</td>".length();
            strResponse = strResponse.substring(nIndex2);
            par.str1 = strResponse;
            par.str2 = strPrice;
            par.str3 = strSeats;
            return bResult;
        }
        //price and seats
        nIndex1 = strSub.indexOf("<p class='price'>");
        if (nIndex1 != -1)
        {
            nIndex1 +=  "<p class='price'>".length();
            strSub = strSub.substring(nIndex1);
            int nLen = strSub.length();
            if (nLen == 0){
                //goto end;
                nIndex2 +=  "</td>".length();
                strResponse = strResponse.substring(nIndex2);
                par.str1 = strResponse;
                par.str2 = strPrice;
                par.str3 = strSeats;
                return bResult;
            }
            for (i = 0; i < nLen; i++)
            {
                char ch = strSub.charAt(i);
                if (ch == '<')
                    break;
                strPrice += ch;
            }
            if (strPrice.length() == 0){
                //goto end;
                nIndex2 +=  "</td>".length();
                strResponse = strResponse.substring(nIndex2);
                par.str1 = strResponse;
                par.str2 = strPrice;
                par.str3 = strSeats;
                return bResult;
            }

            nIndex1 = strSub.indexOf("<p class='seats_avail'>");
            if (nIndex1 == -1){
                //goto end;
                nIndex2 +=  "</td>".length();
                strResponse = strResponse.substring(nIndex2);
                par.str1 = strResponse;
                par.str2 = strPrice;
                par.str3 = strSeats;
                return bResult;
            }

            nIndex1 += "<p class='seats_avail'>".length();
            strSub = strSub.substring(nIndex1);
            nLen = strSub.length();
            if (nLen == 0){
                //goto end;
                nIndex2 +=  "</td>".length();
                strResponse = strResponse.substring(nIndex2);
                par.str1 = strResponse;
                par.str2 = strPrice;
                par.str3 = strSeats;
                return bResult;
            }
            for (i = 0; i < nLen; i++)
            {
                char ch = strSub.charAt(i);
                if (ch == '<')
                    break;
                strSeats += ch;
            }
            if (strSeats.length() == 0)
                bResult = true;
        }

//        end:
        nIndex2 +=  "</td>".length();
        strResponse = strResponse.substring(nIndex2);
        par.str1 = strResponse;
        par.str2 = strPrice;
        par.str3 = strSeats;
        return bResult;
    }
}