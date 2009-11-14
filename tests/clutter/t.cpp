#include <clutter/clutter.h>
#include <cstdlib>

gboolean static on_stage_button_press(ClutterStage * stage, ClutterEvent * event, gpointer data) {
  gint x = 0;
  gint y = 0;
  clutter_event_get_coords (event, &x, &y);
  g_print("stage clicked at (%d, %d)\n", x, y);
  return true; // stops further handling of this event
}

int main(int argc, char * argv[]) {
  clutter_init (&argc, &argv);

  ClutterColor stage_color = { 0x00, 0x00, 0x00, 0xff }; // black

  // get the stage and set its size and color:
  ClutterActor * stage = clutter_stage_get_default ();
  clutter_actor_set_size(stage, 200, 200);
  clutter_stage_set_color(CLUTTER_STAGE(stage), &stage_color);

  // show the stage:
  clutter_actor_show(stage);

  // connect a signal handler to handle mouse clicks and key presses on the stage:
  g_signal_connect(stage, "button-press-event", G_CALLBACK(on_stage_button_press), (void*)(0));

  // start the main loop, so we can respond to events:
  clutter_main();

  return 0;
}
