/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

package com.arm.malideveloper.openglessdk.metaballs;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;

public class Metaballs extends Activity
{
    private static String LOGTAG = "Metaballs";
    protected MetaballsView metaballsView;

    @Override protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        Log.i(LOGTAG, "Creating New Metaballs View");
        metaballsView = new MetaballsView(getApplication());
        setContentView(metaballsView);
    }
    @Override protected void onPause()
    {
        super.onPause();
        metaballsView.onPause();
    }
    @Override protected void onResume()
    {
        super.onResume();
        metaballsView.onResume();
    }
    @Override protected void onDestroy()
    {
        super.onDestroy();
    }
}