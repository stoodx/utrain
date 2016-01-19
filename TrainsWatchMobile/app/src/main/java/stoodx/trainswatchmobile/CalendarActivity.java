package stoodx.trainswatchmobile;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.CalendarView;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

public class CalendarActivity extends Activity {

    private String m_strSelectedDate;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_calendar);

        m_strSelectedDate = "";

        final CalendarView calendarView = (CalendarView)findViewById(R.id.calendarView);
        final Calendar calendar = Calendar.getInstance();

        final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy");
        m_strSelectedDate = sdf.format(new Date(calendarView.getDate()));
        calendarView.setOnDateChangeListener(new CalendarView.OnDateChangeListener(){
            @Override
            public void onSelectedDayChange(CalendarView view, int year, int month, int dayOfMonth) {
                calendar.set(year, month, dayOfMonth);
                long selectedDateInMillis = calendar.getTimeInMillis();
                m_strSelectedDate = sdf.format(selectedDateInMillis);
            }
        });

    }

    public final static String CALENDAR = "stoodx.trainswatchmobile.CALENDAR";

    public void onClickReturnToMainWindow(View view) {

        Intent answerIntent = new Intent();
        answerIntent.putExtra(CALENDAR, m_strSelectedDate);

        setResult(RESULT_OK, answerIntent);
        finish();
    }
}
