package io.github.hexstr.UnityFPSUnlocker;

import android.content.SharedPreferences;
import android.os.Bundle;

import androidx.preference.ListPreference;
import androidx.preference.PreferenceFragmentCompat;

public class PerAppSettingsFragment extends PreferenceFragmentCompat {
    private final SharedPreferences.OnSharedPreferenceChangeListener listener =
            (sharedPreferences, key) -> {
                String packageName = sharedPreferences.getString("package_name", "");
                if (packageName.isEmpty()) {
                    return;
                }

                SharedPreferences destination_preferences = Prefs.getSharedPrefs(requireContext());
                SharedPreferenceCopier.copyAll(
                        sharedPreferences,
                        destination_preferences,
                        packageName + "_");
            };


    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.per_app_preferences, rootKey);

        ListPreference listPreference = findPreference("per_app_display_mode_id");
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
