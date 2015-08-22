/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
Copyright (c) 2000-2014 Torus Knot Software Ltd
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

package org.ogre3d.android;

import android.app.Activity;
import android.graphics.PointF;
import android.graphics.drawable.BitmapDrawable;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.os.Bundle;
import android.os.Handler;
import android.view.Display;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.content.res.AssetManager;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.RelativeLayout;
import org.ogre3d.jni.R;
import android.util.Log;
import android.os.SystemClock;

public class MainActivity extends Activity implements SensorEventListener {
	private int direction_ = 0;

	protected Handler handler = null;
	protected SurfaceView surfaceView = null;
	protected Surface lastSurface = null;

	private Runnable renderer = null;
	private boolean paused = false;
	private boolean initOGRE = false;
	private AssetManager assetMgr = null;

	private JoystickView joystick;
	private FrameLayout controllerContainer;
	private RelativeLayout gameController;
	private PointF shiftDirection, tmp;
	private float shiftAngleHor, shiftAngleVer;

	private Button rightDeckFire = null;
	private Button leftDeckFire = null;

	public boolean isPointInsideView(float x, float y, View view){
		int location[] = new int[2];
		view.getLocationOnScreen(location);
		int viewX = location[0];
		int viewY = location[1];

		//point is inside view bounds
		if(( x > viewX && x < (viewX + view.getWidth())) &&
				( y > viewY && y < (viewY + view.getHeight()))){
			return true;
		} else {
			return false;
		}
	}
	
	private Handler fireButtonClickHandler = new Handler();
	private Runnable leftFireButtonClickRunnable = new Runnable()
	{
            public void run()
            {
                leftDeckFire.setBackgroundResource(R.drawable.leftfirebutton);
                leftDeckFire.setEnabled(true);
            }
	};
	
        private Runnable rightFireButtonClickRunnable = new Runnable()
	{
            public void run()
            {
                rightDeckFire.setBackgroundResource(R.drawable.rightfirebutton);
                rightDeckFire.setEnabled(true);
            }
	};

	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		rightDeckFire = new Button(this);
		leftDeckFire = new Button(this);
		leftDeckFire = new Button(this);
		joystick = new JoystickView(this);
		joystick.initJoystickView();
		controllerContainer = new FrameLayout(this);
		gameController = new RelativeLayout(this);
		handler = new Handler();
		
        joystick.setOnTouchListener(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View v, MotionEvent motionEvent) {
                        joystick.handleTouch(motionEvent);
                        
                        int maskedAction = motionEvent.getActionMasked();
                        int pointerCount = motionEvent.getPointerCount();
                        int actionIndex = motionEvent.getActionIndex();
                        
                        switch(maskedAction)
                        {
                            case MotionEvent.ACTION_POINTER_DOWN:
                                    //Log.e("TRULY", "truly true");
                                    int location[] = new int[2];
                                    joystick.getLocationOnScreen(location);

                                    long downTime = SystemClock.uptimeMillis();
                                    long eventTime = SystemClock.uptimeMillis() + 100;
                                    float x = motionEvent.getX(actionIndex);
                                    float y = motionEvent.getY(actionIndex) + location[1];
                                    int metaState = 0;
                                    Log.e("LOGGING", String.valueOf(x) + " " + String.valueOf(y));
                                    final MotionEvent motionEventTMP = MotionEvent.obtain(
                                        downTime, 
                                        eventTime, 
                                        MotionEvent.ACTION_DOWN, 
                                        x, 
                                        y, 
                                        metaState
                                    );
                                    if (isPointInsideView(motionEventTMP.getX(), motionEventTMP.getY(), leftDeckFire))
                                    {
                                            leftDeckFire.performClick();
                                    }
                                    else if(isPointInsideView(motionEventTMP.getX(), motionEventTMP.getY(), rightDeckFire))
                                    {
                                            rightDeckFire.performClick();
                                    }
                                    else if (!isPointInsideView(motionEventTMP.getX(), motionEventTMP.getY(), joystick))
                                    {

                                    }
                            break;
                        }
                        
                        return true;
                }
        });
		
        leftDeckFire.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                        OgreActivityJNI.shootLeftDeck();
                        leftDeckFire.setBackgroundResource(0);
                        leftDeckFire.setEnabled(false);
                        fireButtonClickHandler.postDelayed(leftFireButtonClickRunnable, 3000);
                }
        });
        
        rightDeckFire.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                        OgreActivityJNI.shootRightDeck();
                        rightDeckFire.setBackgroundResource(0);
                        rightDeckFire.setEnabled(false);
                        fireButtonClickHandler.postDelayed(rightFireButtonClickRunnable, 3000);
                }
        });
		
        joystick.setOnJoystickMoveListener(new JoystickView.OnJoystickMoveListener() {
			@Override
			public void onValueChanged(int angle, int power, int direction) {
				direction_ = direction;
			}
		}, 0);
		shiftDirection = new PointF(0, 0);
		tmp = new PointF(0, 0);
		shiftAngleHor = 0.0f;
		shiftAngleVer = 0.0f;

        Display display = getWindowManager().getDefaultDisplay();
        final int width = display.getWidth();
        final int height = display.getHeight();

		controllerContainer.setOnTouchListener(new View.OnTouchListener() {
			@Override
			public boolean onTouch(View view, MotionEvent motionEvent) {
				int maskedAction = motionEvent.getActionMasked();
				int pointerCount = motionEvent.getPointerCount();

                                int actionIndex = motionEvent.getActionIndex();
                                
                                switch (maskedAction) {
                                        case MotionEvent.ACTION_DOWN:
                                                tmp.x = motionEvent.getX();
                                                tmp.y = motionEvent.getY();
                                                break;
                                        case MotionEvent.ACTION_MOVE:
                                        
                                                int count = motionEvent.getPointerCount();
                                                int pointerId = 0;
                                                for (int i = 0; i < count; i++) {
                                                        pointerId = motionEvent.getPointerId(i);
                                                        if(pointerId == 0)
                                                        {
                                                            shiftDirection.x = motionEvent.getX() - tmp.x;
                                                            shiftDirection.y = motionEvent.getY() - tmp.y;
                                                            tmp.x += (motionEvent.getX() - tmp.x);
                                                            tmp.y += (motionEvent.getY() - tmp.y);
                                                            float mainHor = width;
                                                            float shiftHor = shiftDirection.x;
                                                            shiftAngleHor = shiftHor / mainHor;

                                                            float mainVer = height;
                                                            float shiftVer = shiftDirection.y;
                                                            shiftAngleVer = shiftVer / mainVer;
                                                        }
                                                        
                                                        if (pointerId == 1 && count > 1)
                                                        {
                                                        
                                                            int location[] = new int[2];
                                                            joystick.getLocationOnScreen(location);

                                                            long downTime = SystemClock.uptimeMillis();
                                                            long eventTime = SystemClock.uptimeMillis() + 100;
                                                            float x = motionEvent.getX(1) - location[0];
                                                            float y = motionEvent.getY(1) - location[1];
                                                            int metaState = 0;
                                                            //Log.e("LOGGING", String.valueOf(x) + " " + String.valueOf(y));
                                                            final MotionEvent motionEventTMP = MotionEvent.obtain(
                                                                downTime, 
                                                                eventTime, 
                                                                MotionEvent.ACTION_MOVE, 
                                                                x, 
                                                                y, 
                                                                metaState
                                                            );
                                                            runOnUiThread(new Runnable(){
                                                                public void run()
                                                                {
                                                                    joystick.onTouchEvent(motionEventTMP);
                                                                }
                                                            });
                                                        }
                                                }
                                                        break;

                                                case MotionEvent.ACTION_POINTER_DOWN:
                                                        if (isPointInsideView(motionEvent.getX(actionIndex), motionEvent.getY(actionIndex), leftDeckFire))
                                                        {
                                                                leftDeckFire.performClick();
                                                        }
                                                        else if(isPointInsideView(motionEvent.getX(actionIndex), motionEvent.getY(actionIndex), rightDeckFire))
                                                        {
                                                                rightDeckFire.performClick();
                                                        }
                                                        else if (isPointInsideView(motionEvent.getX(actionIndex), motionEvent.getY(actionIndex), joystick))
                                                        {
                                                            int location[] = new int[2];
                                                            joystick.getLocationOnScreen(location);

                                                            long downTime = SystemClock.uptimeMillis();
                                                            long eventTime = SystemClock.uptimeMillis() + 100;
                                                            float x = motionEvent.getX(1) - location[0];
                                                            float y = motionEvent.getY(1) - location[1];
                                                            int metaState = 0;
                                                            //Log.e("LOGGING", String.valueOf(x) + " " + String.valueOf(y));
                                                            final MotionEvent motionEventTMP = MotionEvent.obtain(
                                                                downTime, 
                                                                eventTime, 
                                                                MotionEvent.ACTION_DOWN, 
                                                                x, 
                                                                y, 
                                                                metaState
                                                            );
                                                            runOnUiThread(new Runnable(){
                                                                public void run()
                                                                {
                                                                    joystick.onTouchEvent(motionEventTMP);
                                                                }
                                                            });
                                                        }
                                                        break;
                                                        
                                                case MotionEvent.ACTION_POINTER_UP:
                                                        int location[] = new int[2];
                                                        joystick.getLocationOnScreen(location);

                                                        long downTime = SystemClock.uptimeMillis();
                                                        long eventTime = SystemClock.uptimeMillis() + 100;
                                                        float x = motionEvent.getX(1) - location[0];
                                                        float y = motionEvent.getY(1) - location[1];
                                                        int metaState = 0;
                                                        //Log.e("LOGGING", String.valueOf(x) + " " + String.valueOf(y));
                                                        final MotionEvent motionEventTMP = MotionEvent.obtain(
                                                            downTime, 
                                                            eventTime, 
                                                            MotionEvent.ACTION_UP, 
                                                            x, 
                                                            y, 
                                                            metaState
                                                        );
                                                        runOnUiThread(new Runnable(){
                                                            public void run()
                                                            {
                                                                joystick.onTouchEvent(motionEventTMP);
                                                            }
                                                        });
                                                        break;

                                                case MotionEvent.ACTION_UP:
                                                        shiftDirection.x = 0;
                                                        shiftDirection.y = 0;
                                                        shiftAngleHor = 0.0f;
                                                        shiftAngleVer = 0.0f;
                                                        break;

                                }

				return true;
			}
		});
		sysInit();
	}

	@Override
	protected void onPause() {
		super.onPause();
		handler.removeCallbacks(renderer);
		paused = true;
	}

	@Override
	protected void onResume() {
		super.onResume();
		paused = false;
		handler.post(renderer);
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();

		Runnable destroyer = new Runnable() {
			public void run() {
				OgreActivityJNI.destroy();
			}
		};
		handler.post(destroyer);
	}

	private void sysInit() {
		final Runnable initRunnable = new Runnable() {
			public void run() {
				if (!initOGRE) {
					initOGRE = true;
					
					if(assetMgr == null) {
						assetMgr = getResources().getAssets();
					}
					
					OgreActivityJNI.create(assetMgr);

					renderer = new Runnable() {
						public void run() {

							if (paused)
								return;

							if (!wndCreate && lastSurface != null) {
								wndCreate = true;
								OgreActivityJNI.initWindow(lastSurface);
								handler.post(this);
								return;
							}

							if (initOGRE && wndCreate) {
								OgreActivityJNI.renderOneFrame(direction_, shiftAngleHor, shiftAngleVer);
							}
                            //Log.e("ENTERED", String.valueOf(joystick.getDirection()));
							handler.post(this);
						}
					};

					handler.post(renderer);
				}
			}

		};

		SurfaceView view = new SurfaceView(this);
		SurfaceHolder holder = view.getHolder();
		// holder.setType(SurfaceHolder.SURFACE_TYPE_GPU);
		surfaceView = view;


		holder.addCallback(new Callback() {
			public void surfaceCreated(SurfaceHolder holder) {
				if (holder.getSurface() != null
						&& holder.getSurface().isValid()) {
					lastSurface = holder.getSurface();
					handler.post(initRunnable);
				}
			}

			public void surfaceDestroyed(SurfaceHolder holder) {
				if (initOGRE && wndCreate) {
					wndCreate = false;
					lastSurface = null;
					handler.post(new Runnable() {
						public void run() {
							OgreActivityJNI.termWindow();
						}
					});
				}
			}

			public void surfaceChanged(SurfaceHolder holder, int format,
									   int width, int height) {

			}
		});

		RelativeLayout.LayoutParams params =
				new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
		params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM, RelativeLayout.TRUE);
		params.addRule(RelativeLayout.ALIGN_PARENT_LEFT, RelativeLayout.TRUE);
		Display display = getWindowManager().getDefaultDisplay();
		int height = display.getHeight();
		params.height = height / 3;
		params.width = height / 3;
		joystick.setLayoutParams(params);

		params = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
		params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM, RelativeLayout.TRUE);
		params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT, RelativeLayout.TRUE);
		params.height = height / 4;
		params.width = height / 8;
		rightDeckFire.setId(1111);
		rightDeckFire.setLayoutParams(params);
		rightDeckFire.setBackgroundResource(R.drawable.rightfirebutton);
		
                params = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
		params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM, RelativeLayout.TRUE);
		params.addRule(RelativeLayout.LEFT_OF, rightDeckFire.getId());
		params.height = height / 4;
		params.width = height / 8;
		leftDeckFire.setLayoutParams(params);
		leftDeckFire.setBackgroundResource(R.drawable.leftfirebutton);

		params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.FILL_PARENT, RelativeLayout.LayoutParams.FILL_PARENT);
		gameController.setLayoutParams(params);
		gameController.addView(joystick);
		gameController.addView(rightDeckFire);
		gameController.addView(leftDeckFire);


		controllerContainer.addView(surfaceView);
		controllerContainer.addView(gameController);
		setContentView(controllerContainer);
	}

	boolean wndCreate = false;

	public void onAccuracyChanged(Sensor sensor, int accuracy) {

	}

	public void onSensorChanged(SensorEvent event) {
		if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
		}
	}

	static {
		System.loadLibrary("OgreJNI");
	}
}