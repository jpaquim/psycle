#include <clutter/clutter.h>
#include <stdlib.h>

ClutterActor *rect = NULL;

/* This must return a value between 0 and CLUTTER_ALPHA_MAX_ALPHA,
 * where 0 means the start of the path, 
 * and CLUTTER_ALPHA_MAX_ALPHA is the end of the path.
 *
 * This will be called as many times per seconds as specified in our call to clutter_timeline_new().
 *
 * See also, for instance CLUTTER_ALPHA_SINE_HALF for a useful built-in callback.
 */
guint32
on_alpha (ClutterAlpha *alpha, gpointer data)
{
  /* Get the position in the timeline, 
   *  so we can base our value upon it:
   */
  ClutterTimeline *timeline = clutter_alpha_get_timeline (alpha);
  const int current_frame_num = clutter_timeline_get_current_frame (timeline);
  const int n_frames = clutter_timeline_get_n_frames (timeline);

  /* Return a value that is simply proportional to the frame position: */
  return CLUTTER_ALPHA_MAX_ALPHA * current_frame_num / n_frames;
}


int main(int argc, char *argv[])
{
  ClutterColor stage_color = { 0x00, 0x00, 0x00, 0xff };
  ClutterColor rect_color = { 0xff, 0xff, 0xff, 0x99 };

  clutter_init (&argc, &argv);

  /* Get the stage and set its size and color: */
  ClutterActor *stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 200, 200);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  /* Add a rectangle to the stage: */
  rect = clutter_rectangle_new_with_color (&rect_color);
  clutter_actor_set_size (rect, 40, 40);
  clutter_actor_set_position (rect, 10, 10);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), rect);
  clutter_actor_show (rect);

  /* Show the stage: */
  clutter_actor_show (stage);

  ClutterTimeline *timeline = clutter_timeline_new(100 /* frames */, 85 /* frames per second. */);
  clutter_timeline_set_loop(timeline, TRUE); 
  clutter_timeline_start(timeline);

  /* Instead of our custom callback, 
   * we could use a standard callback. For instance, CLUTTER_ALPHA_SINE_INC. 
   */
  ClutterEffectTemplate *effect_template = clutter_effect_template_new (timeline, &on_alpha);

  ClutterKnot knot[2];
  knot[0].x = 10;
  knot[0].y = 10;
  knot[1].x= 150;
  knot[1].y= 150;
 
  // Move the actor along the path:
  clutter_effect_path (effect_template, rect, knot, sizeof(knot) / sizeof(ClutterKnot), NULL, NULL);

  // Also change the actor's opacity while moving it along the path:
  // (You would probably want to use a different ClutterEffectTemplate, 
  // so you could use a different alpha callback for this.)
  clutter_effect_fade (effect_template, rect, 50, NULL, NULL);

  g_object_unref (effect_template);
  g_object_unref (timeline);

  /* Start the main loop, so we can respond to events: */
  clutter_main ();

  return EXIT_SUCCESS;

}

