package io.github.hexstr.UnityFPSUnlocker;

import android.content.SharedPreferences;
import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;
import androidx.preference.PreferenceFragmentCompat;

import java.util.Map;

public class SettingsActivity extends AppCompatActivity {
    public static SharedPreferences prefs_ = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.settings_activity);
        if (savedInstanceState == null) {
            getSupportFragmentManager()
                    .beginTransaction()
                    .replace(R.id.settings, new SettingsFragment())
                    .commit();
        }
        prefs_ = Prefs.getSharedPrefs(this);
    }

    public static class SettingsFragment extends PreferenceFragmentCompat {
        SharedPreferences.OnSharedPreferenceChangeListener listener =
                (sharedPreferences, key) -> {
                    Map<String, ?> allEntries = sharedPreferences.getAll();
                    SharedPreferences.Editor editor = SettingsActivity.prefs_.edit();

                    for (Map.Entry<String, ?> entry : allEntries.entrySet()) {
                        String entry_key = entry.getKey();
                        Object entry_value = entry.getValue();

                        if (entry_value instanceof String) {
                            editor.putString(entry_key, (String) entry_value);
                        } else if (entry_value instanceof Integer) {
                            editor.putInt(entry_key, (Integer) entry_value);
                        } else if (entry_value instanceof Boolean) {
                            editor.putBoolean(entry_key, (Boolean) entry_value);
                        }
                    }

                    editor.apply();
                };

        @Override
        public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
            setPreferencesFromResource(R.xml.root_preferences, rootKey);
        }

        @Override
        public void onResume() {
            super.onResume();
            getPreferenceManager().getSharedPreferences().registerOnSharedPreferenceChangeListener(listener);

        }

        @Override
        public void onPause() {
            super.onPause();
            getPreferenceManager().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(listener);
        }
    }
}