package io.github.hexstr.UnityFPSUnlocker;

import android.content.Intent;
import android.os.Bundle;

import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;

public class SettingsFragment extends PreferenceFragmentCompat {
    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        getPreferenceManager().setSharedPreferencesName(Prefs.prefs_name_);
        setPreferencesFromResource(R.xml.root_preferences, rootKey);

        Preference per_app_settings_preference = findPreference("per_app_settings");
        if (per_app_settings_preference != null) {
            per_app_settings_preference.setOnPreferenceClickListener(preference -> {
                Intent intent = new Intent(getActivity(), PerAppSettingsActivity.class);
                startActivity(intent);
                return true;
            });
        }

        ListPreference list_preference = findPreference("display_mode_id");
        DisplayModePreferenceHelper.populate(requireActivity(), list_preference);
    }
}
