/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Copyright (C) 2015 Joey Ferwerda
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Android Driver */

#include "android.h"

typedef struct {
	ohmd_device base;
	fusion sensor_fusion;
} android_priv;

static void update_device(ohmd_device* device)
{
}

static int getf(ohmd_device* device, ohmd_float_value type, float* out)
{
	android_priv* priv = (android_priv*)device;

	switch(type){
		case OHMD_ROTATION_QUAT: {
				*(quatf*)out = priv->sensor_fusion.orient;
				break;
			}

		case OHMD_POSITION_VECTOR:
			out[0] = out[1] = out[2] = 0;
			break;

		case OHMD_DISTORTION_K:
			// TODO this should be set to the equivalent of no distortion
			memset(out, 0, sizeof(float) * 6);
			break;

		default:
			ohmd_set_error(priv->base.ctx, "invalid type given to getf (%d)", type);
			return -1;
			break;
	}

	return 0;
}

static int setf(ohmd_device* device, ohmd_float_value type, float* in)
{
	android_priv* priv = (android_priv*)device;

	switch(type){
		case OHMD_EXTERNAL_SENSOR_FUSION: {
				ofusion_update(&priv->sensor_fusion, *in, (vec3f*)(in + 1), (vec3f*)(in + 4), (vec3f*)(in + 7));
			}
			break;

		default:
			ohmd_set_error(priv->base.ctx, "invalid type given to setf (%d)", type);
			return -1;
			break;
	}

	return 0;
}

static void close_device(ohmd_device* device)
{
	LOGD("closing Android device");
	free(device);
}

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	android_priv* priv = ohmd_alloc(driver->ctx, sizeof(android_priv));
	if(!priv)
		return NULL;

	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);

	// Set device properties
	//TODO: Get information from android about device
	//TODO: Use profile string to set default for a particular device (Durovis, VR One etc)
	priv->base.properties.hsize = 0.149760f;
	priv->base.properties.vsize = 0.093600f;
	priv->base.properties.hres = 1280;
	priv->base.properties.vres = 800;
	priv->base.properties.lens_sep = 0.063500;
	priv->base.properties.lens_vpos = 0.046800;
	priv->base.properties.fov = DEG_TO_RAD(125.5144f);
	priv->base.properties.ratio = (1280.0f / 800.0f) / 2.0f;

	// calculate projection eye projection matrices from the device properties
	ohmd_calc_default_proj_matrices(&priv->base.properties);

	// set up device callbacks
	priv->base.update = update_device;
	priv->base.close = close_device;
	priv->base.getf = getf;
	priv->base.setf = setf;

	ofusion_init(&priv->sensor_fusion);

	return (ohmd_device*)priv;
}

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
	ohmd_device_desc* desc = &list->devices[list->num_devices++];

	strcpy(desc->driver, "OpenHMD Generic Android Driver");
	strcpy(desc->vendor, "OpenHMD");
	strcpy(desc->product, "Android Device");

	strcpy(desc->path, "(none)");

	desc->driver_ptr = driver;
}

static void destroy_driver(ohmd_driver* drv)
{
	LOGD("shutting down Android driver");
	free(drv);
}

ohmd_driver* ohmd_create_android_drv(ohmd_context* ctx)
{
	ohmd_driver* drv = ohmd_alloc(ctx, sizeof(ohmd_driver));
	if(!drv)
		return NULL;

	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->destroy = destroy_driver;

	return drv;
}

/* Android specific functions */
static void set_android_properties(ohmd_driver* driver, ohmd_device_properties* props)
{
    android_priv* priv = (android_priv*)device;

	priv->base.properties.hsize = props->hsize;
	priv->base.properties.vsize = props->vsize;
	priv->base.properties.hres = props->hres;
	priv->base.properties.vres = props->vres;
	priv->base.properties.lens_sep = props->lens_sep;
	priv->base.properties.lens_vpos = props->lens_vpos;
	priv->base.properties.fov = DEG_TO_RAD(props->fov);
	priv->base.properties.ratio = props->ratio;
}

static void set_android_profile(ohmd_driver* driver, android_hmd_profile profile)
{
    switch(profile){
        case DROID_DUROVIS_OPEN_DIVE: break;
        case DROID_DUROVIS_DIVE_5: break;
        case DROID_DUROVIS_DIVE_7: break;
        case DROID_CARL_ZEISS_VRONE: break;
        case DROID_GOOGLE_CARDBOARD: break;
        case DROID_NONE:
        default: break;
    }
}
