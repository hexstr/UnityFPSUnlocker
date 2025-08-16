package io.github.hexstr.UnityFPSUnlocker;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.view.Display;
import android.view.WindowManager;

import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;

import java.util.Map;

public class SettingsFragment extends PreferenceFragmentCompat {
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
        Preference perAppSettingsPreference = findPreference("per_app_settings");
        if (perAppSettingsPreference != null) {
            perAppSettingsPreference.setOnPreferenceClickListener(preference -> {
                Intent intent = new Intent(getActivity(), PerAppSettingsActivity.class);
                startActivity(intent);
                return true;
            });
        }

        ListPreference listPreference = findPreference("display_mode_id");

        Display display = null;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            display = getActivity().getDisplay();
        } else {
            final WindowManager windowManager = (WindowManager) getActivity().getSystemService(Context.WINDOW_SERVICE);
            display = windowManager.getDefaultDisplay();
        }

        Display.Mode[] modes = display.getSupportedModes();

        String[] entries = new String[modes.length];
        String[] entryValues = new String[modes.length];

        for (int i = 0; i < modes.length; i++) {
            Display.Mode mode = modes[i];
            entries[i] = String.format("%dx%d@%.2f",
                    mode.getPhysicalWidth(),
                    mode.getPhysicalHeight(),
                    mode.getRefreshRate());
            entryValues[i] = String.valueOf(mode.getModeId());
        }

        listPreference.setEntries(entries);
        listPreference.setEntryValues(entryValues);
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
