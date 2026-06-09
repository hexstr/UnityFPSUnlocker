package io.github.hexstr.UnityFPSUnlocker;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;

import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;

public class SettingsFragment extends PreferenceFragmentCompat {
    private final SharedPreferences.OnSharedPreferenceChangeListener listener =
            (sharedPreferences, key) -> {
                SharedPreferences destination_preferences = Prefs.getSharedPrefs(requireContext());
                SharedPreferenceCopier.copyAll(sharedPreferences, destination_preferences, "");
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
        DisplayModePreferenceHelper.populate(requireActivity(), listPreference);
    }

    @Override
    public void onResume() {
        super.onResume();
        SharedPreferences preferences = getPreferenceManager().getSharedPreferences();
        if (preferences != null) {
            preferences.registerOnSharedPreferenceChangeListener(listener);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        SharedPreferences preferences = getPreferenceManager().getSharedPreferences();
        if (preferences != null) {
            preferences.unregisterOnSharedPreferenceChangeListener(listener);
        }
    }
}
