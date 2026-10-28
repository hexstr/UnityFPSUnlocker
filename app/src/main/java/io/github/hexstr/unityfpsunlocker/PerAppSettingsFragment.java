package io.github.hexstr.UnityFPSUnlocker;

import android.content.SharedPreferences;
import android.os.Bundle;

import androidx.preference.ListPreference;
import androidx.preference.PreferenceFragmentCompat;

public class PerAppSettingsFragment extends PreferenceFragmentCompat {
    private final SharedPreferences.OnSharedPreferenceChangeListener listener_ =
            (shared_preferences, key) -> {
                String package_name = shared_preferences.getString("package_name", "");
                if (package_name.isEmpty()) {
                    return;
                }

                SharedPreferences destination_preferences = Prefs.getSharedPrefs(requireContext());
                SharedPreferenceCopier.copyAll(
                        shared_preferences,
                        destination_preferences,
                        package_name + "_");
            };

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.per_app_preferences, rootKey);

        ListPreference list_preference = findPreference("per_app_display_mode_id");
        DisplayModePreferenceHelper.populate(requireActivity(), list_preference);
    }

    @Override
    public void onResume() {
        super.onResume();
        SharedPreferences preferences = getPreferenceManager().getSharedPreferences();
        if (preferences != null) {
            preferences.registerOnSharedPreferenceChangeListener(listener_);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        SharedPreferences preferences = getPreferenceManager().getSharedPreferences();
        if (preferences != null) {
            preferences.unregisterOnSharedPreferenceChangeListener(listener_);
        }
    }
}
