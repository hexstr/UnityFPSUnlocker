package io.github.hexstr.UnityFPSUnlocker;

import android.view.View;

import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

final class SettingsInsets {
    private SettingsInsets() {
    }

    static void apply(View view) {
        int initial_left = view.getPaddingLeft();
        int initial_top = view.getPaddingTop();
        int initial_right = view.getPaddingRight();
        int initial_bottom = view.getPaddingBottom();

        ViewCompat.setOnApplyWindowInsetsListener(view, (target, window_insets) -> {
            Insets system_bars = window_insets.getInsets(WindowInsetsCompat.Type.systemBars());
            target.setPadding(
                    initial_left + system_bars.left,
                    initial_top + system_bars.top,
                    initial_right + system_bars.right,
                    initial_bottom + system_bars.bottom);
            return window_insets;
        });
        ViewCompat.requestApplyInsets(view);
    }
}
