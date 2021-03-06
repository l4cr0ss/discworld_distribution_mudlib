/* this is the third template room, introducing 'signs' and 'objects' -- bil */

/* I will not discuss what has already been discussed...revise the older
 * rooms if you have problems.
 */
#include "path.h"

inherit "/std/room";

void setup()
{
  set_short("third simple room");

  set_long("This is the third example room. Will you ever get to the end? " +
           "Directly above your head is a plaque, nailed to the wall.\n");
  
  set_light(90); /* another brightly lit room */

/* >> Signs <<
 * A sign is a special type of item. It can be read and may be visible in the
 * room, but is usually referenced in the long, just like items.
 * arguments are as follows:
 * 1) description, just like the second arg of items.
 * 2) read message.
 * 3) optional short description - if this is 0 then the sign is treated
 *    like an item, otherwise it is an actual object in the room. Either
 *    way it is auto-destructed for you so don't worry about.
 * 4) optional name - if not set it defaults to 'sign'
 */
  add_sign("The plaque is made of bronze.\n",
           "'Don't read me, I'm only an example sign you know!'\n",
           0,
           "plaque");

  add_exit("west", ROOM + "exa2", "door");
  add_exit("east", ROOM + "exa4", "door");

  set_zone("examples");
}

/* >> reset <<
 * OooooOoOoOoOoOOooOoooh, a new function!
 * the reset function is called every so often, and whenever an object is
 * created. It is generally used to restock the game with monsters and objects.
 * It is the equivalent of reset(1) for the LP-2.4.5 people out there.
 */
void reset()
{
/* local variable, just used to reference an object we clone while we set it up
 */
  object bar;
  
/* oooh checking if an item is there...horror stuff.
 * read the help on match_objects_for_existence, sizeof and this_object and come back later *8-)*
 * what we are doing now is checking to see is there is already
 * a bar here, and if so we do not continue with making a new one.
 * Why there would be a new one I haven't a clue, and you should
 * never actually just give away items like this. So why am I doing it?
 * It does teach you some things. Bing!
 */
  if(sizeof(match_objects_for_existence("copper bar", ({ this_object() }) )))
    return;
/* cloning objects - here we clone a standard object ("/std/object") and
 * set the variable 'bar' to point to it.
 */
  bar = clone_object("/std/object");
/* here we initialise the data in the object. We do this by calling functions
 * on the bar, the -> operator means 'call_other'.
 * first we set the most simple name the object is called.
 * names should be ONE word long and in LOWER CASE.
 * everything except short and long should be in LOWER CASE.
 * the name is used so that when someone does 'examine bar' we will
 * know what they are talking about.
 */
  bar->set_name("bar");

/* adding the adjective "copper" to the bar means that both
 * "bar" and "copper bar" can be used to identify the object.
 */
  bar->add_adjective("copper");
/* the main plural replaces the short description when there are multiple
 * objects with the same short description in the same place.
 */
  bar->set_main_plural("copper bars");

/* if there are other ways to identify a group of the objects add it
 * with add_plural. If the argument is an array of strings they are all
 * added as extra plural identifiers.
 */
  bar->add_plural("bars");
/* now a group of the object can be identified as "copper bars" or
 * "bars".
 */
 
/* short and long have already been discussed in first example room
 * with respect to rooms. The short is what you see in your inventory
 * or when the thing is on the ground. You should not use a preceeding
 * 'a' or 'a' or 'the'.
 */
  bar->set_short("copper bar");
  bar->set_long("This is just a treasure thingie to show you how to use " +
                  "such fun things.\n");

/* Weight - this is how much the player is encumbered by the item.
 * it is measured in arbitary units and you can get an idea of how
 * much real weight they corrispond to with 'help weight' eg. bar is 4kg
 */
  bar->set_weight(80);

/* Value - this means the absolute value of the bar is 2000 brass coins
 * or 1 gold coin (200 copper coins) - remember this is the shop
 * price - a player will generally get half this when selling the item.
 * look at the exchange rates in 'help money'.
 */
  bar->set_value(2000);

/* So far the object isn't anywhere, it has no environment. to move it
 * call the move() function in it to move it to this_object() (the room)
 */
  bar->move(this_object());
}
