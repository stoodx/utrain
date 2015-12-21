package stoodx.trainswatchmobile;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //From
        Spinner spinnerFromA = (Spinner)findViewById(R.id.spinnerFromA);
        ArrayAdapter<CharSequence> adapterFromA = ArrayAdapter.createFromResource(this,
                R.array.spinnerFromA, android.R.layout.simple_spinner_item);
        adapterFromA.setDropDownViewResource(android.R.layout.simple_spinner_item);
        spinnerFromA.setAdapter(adapterFromA);

        //To
        Spinner spinnerToA = (Spinner) findViewById(R.id.spinnerToA);
        ArrayAdapter<CharSequence> adapterToA = ArrayAdapter.createFromResource(this,
                R.array.spinnerToA, android.R.layout.simple_spinner_item);
        adapterToA.setDropDownViewResource(android.R.layout.simple_spinner_item);
        spinnerToA.setAdapter(adapterToA);
    }
}
