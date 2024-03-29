/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2010 Sreerenj Balachandran <bsreerenj@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/**
 * SECTION:element-opencvtextoverlay
 *
 * opencvtextoverlay renders the text on top of the video frames
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 videotestsrc ! videoconvert ! opencvtextoverlay text="Opencv Text Overlay " ! videoconvert ! xvimagesink
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gststeampunkgoggles.h"
#include "zhelpers.h"
#include "lib/cppprocess.h"

GST_DEBUG_CATEGORY_STATIC (gst_opencv_steampunk_goggles_debug);
#define GST_CAT_DEFAULT gst_opencv_opencv_steampunk_goggles_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("RGB"))
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("RGB"))
    );

G_DEFINE_TYPE (GstOpenCVSteamPunkGoggles, gst_opencv_steampunk_goggles,
    GST_TYPE_OPENCV_VIDEO_FILTER);

// static void gst_opencv_steampunk_goggles_set_property (GObject * object,
//     guint prop_id, const GValue * value, GParamSpec * pspec);
// static void gst_opencv_steampunk_goggles_get_property (GObject * object,
//     guint prop_id, GValue * value, GParamSpec * pspec);

static GstFlowReturn gst_opencv_steampunk_goggles_transform_ip (GstOpencvVideoFilter
    * filter, GstBuffer * buf, IplImage * img);

/* Clean up */
static void
gst_opencv_steampunk_goggles_finalize (GObject * obj)
{
  GstOpenCVSteamPunkGoggles *filter = GST_OPENCV_STEAMPUNK_GOGGLES (obj);

  zmq_close (filter->publisher);
  zmq_ctx_destroy (filter->context);
  free(filter->data);

  G_OBJECT_CLASS (gst_opencv_steampunk_goggles_parent_class)->finalize (obj);
}

/* initialize the opencvtextoverlay's class */
static void
gst_opencv_steampunk_goggles_class_init (GstOpenCVSteamPunkGogglesClass * klass)
{
  GObjectClass *gobject_class;
  GstOpencvVideoFilterClass *gstopencvbasefilter_class;
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gobject_class = (GObjectClass *) klass;
  gobject_class->finalize =
      GST_DEBUG_FUNCPTR (gst_opencv_steampunk_goggles_finalize);
  gstopencvbasefilter_class = (GstOpencvVideoFilterClass *) klass;

  gstopencvbasefilter_class->cv_trans_ip_func =
      gst_opencv_steampunk_goggles_transform_ip;

/*
  gobject_class->set_property = gst_opencv_steampunk_goggles_set_property;
  gobject_class->get_property = gst_opencv_steampunk_goggles_get_property;

  g_object_class_install_property (gobject_class, PROP_TEXT,
      g_param_spec_string ("text", "text",
          "Text to be display.", DEFAULT_PROP_TEXT,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_XPOS,
      g_param_spec_int ("xpos", "horizontal position",
          "Sets the Horizontal position", 0, G_MAXINT,
          DEFAULT_PROP_XPOS,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_YPOS,
      g_param_spec_int ("ypos", "vertical position",
          "Sets the Vertical position", 0, G_MAXINT,
          DEFAULT_PROP_YPOS,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_THICKNESS,
      g_param_spec_int ("thickness", "font thickness",
          "Sets the Thickness of Font", 0, G_MAXINT,
          DEFAULT_PROP_THICKNESS,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_COLOR_R,
      g_param_spec_int ("colorR", "color -Red ",
          "Sets the color -R", 0, 255,
          DEFAULT_PROP_COLOR,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_COLOR_G,
      g_param_spec_int ("colorG", "color -Green",
          "Sets the color -G", 0, 255,
          DEFAULT_PROP_COLOR,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_COLOR_B,
      g_param_spec_int ("colorB", "color -Blue",
          "Sets the color -B", 0, 255,
          DEFAULT_PROP_COLOR,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_HEIGHT,
      g_param_spec_double ("height", "Height",
          "Sets the height of fonts", 1.0, 5.0,
          DEFAULT_HEIGHT,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_WIDTH,
      g_param_spec_double ("width", "Width",
          "Sets the width of fonts", 1.0, 5.0,
          DEFAULT_WIDTH,
          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
*/
  gst_element_class_set_static_metadata (element_class,
      "opencvsteampunkgoggles",
      "Filter/Effect/Video",
      "Custom FIRST Steamworks video filter", "warrenlp<parsonswarren30@gmail.com>");

  gst_element_class_add_static_pad_template (element_class, &src_factory);
  gst_element_class_add_static_pad_template (element_class, &sink_factory);

}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_opencv_steampunk_goggles_init (GstOpenCVSteamPunkGoggles * filter)
{

  printf("gst_opencv_steampunk_goggles_init\n");
  // Set ContourFilter values
  filter->contourFilters.minArea = 1.0;
  filter->contourFilters.maxArea = 50000.0;
  filter->contourFilters.minPerimeter = 50.0;
  filter->contourFilters.minWidth = 20.0;
  filter->contourFilters.minHeight = 40.0;
  filter->contourFilters.maxHeight = 1500.0;
  filter->contourFilters.maxWidth = 200.0;

  filter->context = zmq_ctx_new();
  filter->publisher = zmq_socket (filter->context, ZMQ_PUB);
  filter->rc = zmq_bind (filter->publisher, "tcp://*:5800");
  assert (filter->rc == 0);
  filter->data = malloc(sizeof(int));

  gst_opencv_video_filter_set_in_place (GST_OPENCV_VIDEO_FILTER_CAST (filter),
      TRUE);
}

/*
 static void
 gst_opencv_steampunk_goggles_set_property (GObject * object, guint prop_id,
     const GValue * value, GParamSpec * pspec)
 {
   GstOpenCVSteamPunkGoggles *filter = GST_OPENCV_STEAMPUNK_GOGGLES (object);

   switch (prop_id) {
     case PROP_TEXT:
       g_free (filter->textbuf);
       filter->textbuf = g_value_dup_string (value);
       break;
     case PROP_XPOS:
       filter->xpos = g_value_get_int (value);
       break;
     case PROP_YPOS:
       filter->ypos = g_value_get_int (value);
       break;
     case PROP_THICKNESS:
       filter->thickness = g_value_get_int (value);
       break;

     case PROP_COLOR_R:
       filter->colorR = g_value_get_int (value);
       break;
     case PROP_COLOR_G:
       filter->colorG = g_value_get_int (value);
       break;
     case PROP_COLOR_B:
       filter->colorB = g_value_get_int (value);
       break;

     case PROP_HEIGHT:
       filter->height = g_value_get_double (value);
       break;
     case PROP_WIDTH:
       filter->width = g_value_get_double (value);
       break;
     default:
       G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
       break;
   }
 }
 */

/*
 static void
 gst_opencv_steampunk_goggles_get_property (GObject * object, guint prop_id,
     GValue * value, GParamSpec * pspec)
 {
   GstOpenCVSteamPunkGoggles *filter = GST_OPENCV_STEAMPUNK_GOGGLES (object);

   switch (prop_id) {
     case PROP_TEXT:
       g_value_set_string (value, filter->textbuf);
       break;
     case PROP_XPOS:
       g_value_set_int (value, filter->xpos);
       break;
     case PROP_YPOS:
       g_value_set_int (value, filter->ypos);
       break;
     case PROP_THICKNESS:
       g_value_set_int (value, filter->thickness);
       break;
     case PROP_COLOR_R:
       g_value_set_int (value, filter->colorR);
       break;
     case PROP_COLOR_G:
       g_value_set_int (value, filter->colorG);
       break;
     case PROP_COLOR_B:
       g_value_set_int (value, filter->colorB);
       break;
     case PROP_HEIGHT:
       g_value_set_double (value, filter->height);
       break;
     case PROP_WIDTH:
       g_value_set_double (value, filter->width);
       break;
     default:
       G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
       break;
   }
 }
 */

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_opencv_steampunk_goggles_transform_ip (GstOpencvVideoFilter * base,
    GstBuffer * buf, IplImage * img)
{
    GstOpenCVSteamPunkGoggles *filter = GST_OPENCV_STEAMPUNK_GOGGLES (base);

    // This is where the OpenCV processing magic happens
    *(filter->data) = cppProcess(img);

    char update[20];
    sprintf(update, "CenterX: %d\n", *(filter->data));

    s_send (filter->publisher, update);

    return GST_FLOW_OK;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
gboolean
gst_opencv_steampunk_goggles_plugin_init (GstPlugin * plugin)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template opencvtextoverlay' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_opencv_steampunk_goggles_debug, "opencvsteampunkgoggles",
      0, "Template opencvsteampunkgoggles");

  return gst_element_register (plugin, "opencvsteampunkgoggles", GST_RANK_NONE,
      GST_TYPE_OPENCV_STEAMPUNK_GOGGLES);
}

#ifndef PACKAGE
#define PACKAGE "opencvsteampunkgoggles"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    opencvsteampunkgoggles,
    "GStreamer OpenCV Steampunk Goggles Plugins",
    gst_opencv_steampunk_goggles_plugin_init,
    "1.0", "LGPL", "GStreamer", "http://gstreamer.net/")
