/*  -*- LPC -*-  */
/*
 * $Locker:  $
 * $Id: remove.c,v 1.19 2003/01/30 06:33:24 pinkfish Exp $
 * $Log: remove.c,v $
 * Revision 1.19  2003/01/30 06:33:24  pinkfish
 * Add in stuff to make sure they drop extra items.
 *
 * Revision 1.18  2002/06/18 20:51:06  presto
 * You know, I really think I might have it right this time.  No, really!
 *
 * Revision 1.17  2002/05/17 00:45:19  trilogy
 *  Forcibly unlocked by presto
 *
 * Revision 1.15  2002/05/16 01:54:40  trilogy
 *  Forcibly unlocked by presto
 *
 * Revision 1.13  2002/02/12 04:42:29  presto
 * Fixed so that an item never blocks itself (which is possible for items
 * of mixed type)
 *
 * Revision 1.12  2002/01/15 01:40:24  presto
 * So help me God, I hope it works now.
 *
 * Revision 1.11  2002/01/07 05:27:16  presto
 * Fixed mistake with not removing things that were *indirectly* blocked by
 * other things
 *
 * Revision 1.10  2002/01/06 05:37:27  presto
 * Rewrote.  Seems to work better now, and I think it's more understandable. :)
 * Will keep an eye on it
 *
 * Revision 1.9  2001/12/07 15:12:11  taffyd
 * Fixed typo and grammar oddity.
 *
 * Revision 1.8  2001/11/08 02:22:07  pinkfish
 * Fox up some little errors.
 *
 * Revision 1.7  2001/11/07 23:09:56  pinkfish
 * Fix up the auto removal a little more.
 *
 * Revision 1.6  2001/11/07 21:46:34  pinkfish
 * Make it so it automatically takes and puts stuff on and off.
 *
 * Revision 1.5  1999/12/08 04:09:04  ceres
 * Modified to use standard can_wear_or_remove() function in /obj/handlers/clot
 * hing_handler
 *
 * Revision 1.4  1998/09/15 00:28:32  pinkfish
 * Fix up the output errors for remove.
 *
 * Revision 1.3  1998/08/25 10:18:41  pinkfish
 * Change remove so it will order the thnigs to remove...  SO it takes
 * the toip off first.
 *
 * Revision 1.2  1998/03/27 20:49:01  pinkfish
 * To the new clothing handler.
 *
 * Revision 1.1  1998/01/06 05:28:43  ceres
 * Initial revision
 * 
*/
#include <clothing.h>
#include <tasks.h>

//#define DEBUG

#define SKILL "covert.manipulation.sleight-of-hand"
#ifdef DEBUG
#define TELL_ME "presto"
#endif

inherit "cmds/base";

int cmd(object *things)  {
   object  *removed = ({ });
   object  *blocking = ({ });
   object  *blocked;
   object  *total_blocking = ({ });
   object  *succeeded;
   object  *failed;
   object  *failed_rewear;
   object   ob;
   object   blocker;
   mapping  is_blocking = ([ ]);
   mapping  hide_invis;
   string   tmp1;
   string   tmp2;
   int      last_blocking;
   int      limit;
   int      i;
   int      hiding;
   int      sneaking;
   int      difficulty;
   int      light;
   int      my_light;

   succeeded = this_player()->query_wearing();
   failed = filter(things, (: member_array($1, $(succeeded)) == -1 :));
   things -= failed;
   if (sizeof(things) == 0)  {
      write("You are not wearing " + query_multiple_short(failed, "the") +
            ".\n");
      return 1;
   }

   succeeded = things;
   for (i = 0; i < sizeof(succeeded); i++)  {
      ob = succeeded[i];
      blocking = CLOTHING_HANDLER->query_items_blocking(ob, this_player()) -
                 ({ ob });
      if (sizeof(blocking))  {
         foreach (blocker in blocking)  {
            if (undefinedp(is_blocking[blocker]))
               is_blocking[blocker] = ({ ob });
            else
               is_blocking[blocker] |= ({ ob });
         }

         total_blocking |= blocking;

         /*
          * We need to add the blocking things to the things we are checking,
          * because the blocking things may themselves be blocked by something
          * that was not blocking the thing that the original blocker was
          * blocking.  Oh... you want an EXAMPLE.  OK:
          *
          * Example: a silk shirt is blocked by a mail shirt, but NOT by a
          * scabbard.  However, the mail shirt IS blocked by the scabbard, so
          * it has to be removed before we can remove the mail shirt; so that
          * we can remove the silk shirt.
          */

         succeeded = ({ things..., total_blocking... });
      }
   }

   /*
    * OK, here is the important, complicated bit.  At this point, the
    * is_blocking mapping is keeping track of everything that is blocking
    * something else, and what it is blocking.  We need to build an array of
    * blocking objects in order from outermost layer to innermost.  That way
    * there should be no failures when we remove the covering stuff to get
    * to the things that the player wants to remove.  Here is how this works:
    *
    * 1. Loop over all items that are blocking something else
    * 2. For each item, find the last thing in the array (so far) that would
    *    block the item.  Insert the item we are adding should immediately
    *    after that position.
    * 3. Loop over the array we are building up to the step 2 index and check
    *    whether each object would have been blocked by the new item, or by
    *    anything else that we moved already in a previous run through the
    *    loop.  If so, move the object after the new item and previously moved
    *    items.
    */
    
   total_blocking = ({ });
   foreach (ob, blocked in is_blocking)  {

#ifdef DEBUG
      if (this_player() == find_player(TELL_ME))
      tell_creator(TELL_ME, "ob == %s, blocked == %O\n",
                   ob->short(), blocked->short());
#endif
        
      /* Find the last thing that is blocking 'ob' */
      last_blocking = -1;
      for (i = sizeof(total_blocking) - 1; i >= 0; i--)  {
         if (member_array(ob, is_blocking[total_blocking[i]]) > -1)  {
            last_blocking = i;
            break;
         }
      }

      if (last_blocking == -1)  {
         /*
          * Nothing is blocking 'ob', so we can safely put it at the
          * front of the list, because then, even if it blocks something
          * else, it will get removed first.
          */
         total_blocking = ({ ob, total_blocking... });

#ifdef DEBUG
         if (this_player() == find_player(TELL_ME))
            tell_creator(TELL_ME, "Nothing is blocking ob, adding it to "
                         "the beginning\n%O\n", total_blocking->short());
#endif

         continue;
      }

      total_blocking = ({ total_blocking[0 .. last_blocking]...,
                          ob,
                          total_blocking[(last_blocking + 1) .. ]... });

#ifdef DEBUG
      if (this_player() == find_player(TELL_ME))
         tell_creator(TELL_ME, "Adding ob after last_blocking position "
                      "(%d)\n%O\n",
                      last_blocking, total_blocking->short());
#endif

      limit = last_blocking;
      blocking = copy(blocked);
      for (i = 0; i < limit; i++)  {
         /*
          * If the object we are checking is blocked by anything we have
          * moved so far, then we have to move it too
          */
         if (member_array(total_blocking[i], blocking) > -1)  {
            blocking += is_blocking[total_blocking[i]];
            if (i == 0)
               total_blocking =
                  ({ total_blocking[1 .. (last_blocking + 1)]...,
                     total_blocking[0],
                     total_blocking[(last_blocking + 2) .. ]... });
            else
               total_blocking =
                  ({ total_blocking[0 .. (i - 1)]...,
                     total_blocking[(i + 1) .. (last_blocking + 1)]...,
                     total_blocking[i],
                     total_blocking[(last_blocking + 2) .. ]... });
            --limit;
            --i;

#ifdef DEBUG
            if (this_player() == find_player(TELL_ME))
               tell_creator(TELL_ME, "Reordered list:\n%O\n",
                            total_blocking->short());
#endif

         }
      }
   }

   /* Remove stuff that's blocking the stuff we asked to remove */
   foreach (blocker in total_blocking)  {
      tmp1 = CLOTHING_HANDLER->can_wear_or_remove(blocker, this_player());
      if (tmp1)  { /* This *shouldn't* happen */
//         log_file("REMOVE_FAILURE", "things == %O\n", things);
//         log_file("REMOVE_FAILURE", "reason == %s\n", tmp1);
//         log_file("REMOVE_FAILURE", "blocker == %s (%s, %O)\n", blocker->short(), file_name(blocker), blocker);
//         log_file("REMOVE_FAILURE", "total_blocking == \n");
//         for (i = 0; i < sizeof(total_blocking); i++)  {
//            log_file("REMOVE_FAILURE", "   %O (%s)\n", total_blocking[i], total_blocking[i]->short());
//         }
//         log_file("REMOVE_FAILURE", "is_blocking == \n");
//         foreach (ob, blocked in is_blocking)  {
//            log_file("REMOVE_FAILURE", "   %O (%s): \n", ob, ob->short());
//            for (i = 0; i < sizeof(blocked); i++)  {
//               log_file("REMOVE_FAILURE", "      %O (%s)\n", blocked[i], blocked[i]->short());
//            }
//         }
//         log_file("REMOVE_FAILURE", "removed == %O\n", removed);
         write("You cannot remove " +
               query_multiple_short(is_blocking[blocker], "the") +
               " " + tmp1 + ".\n");
         things -= is_blocking[blocker];
         break;
      }
      else if (this_player()->remove_armour(blocker))  {
         write("You cannot remove " +
               query_multiple_short(is_blocking[blocker], "the") +
               " because you cannot remove " + blocker->one_short() + ".\n");
         things -= is_blocking[blocker];
         break;
      }
      else removed += ({ blocker });
   }

   /* Now remove the stuff we really wanted to */
   /*
    * Start with things we already removed because they were blocking
    * something else
    */
   succeeded = things & removed;
   failed = ({ });

   /* Now try to remove the rest */
   foreach (ob in things - removed)  {
      if (this_player()->remove_armour(ob))
         failed += ({ ob });
      else
         succeeded += ({ ob });
   }

   if (sizeof(succeeded) > 0)  {
      removed -= things;
      tmp2 = query_multiple_short(succeeded, "the") ;
      if (sizeof(removed) > 0)  {
         tmp1 = query_multiple_short(removed, "the");
         write("You remove " + tmp1 + " so you can remove " + tmp2 + ".\n");
         say(this_player()->the_short() + " removes " + tmp1 + " so " +
             this_player()->query_pronoun() + " can remove " + tmp2 + ".\n");
      }
      else  {
        hide_invis = ( mapping )this_player()->query_hide_invis();
        hiding = hide_invis[ "hiding" ] ? 1 : 0;
        sneaking = this_player()->query_sneak_level() ? 1 : 0;
      
        if( hiding || sneaking ) {
          my_light = this_player()->query_light();
          light = environment( this_player() )->query_light();
        
          difficulty = light + ( 4 * my_light ) / ( light + 1 );
      
          difficulty += succeeded[0]->query_complete_weight();
      
          debug_printf( "Difficulty = %d.\n Skill = %s\n Bonus = %d\n",
                        difficulty, SKILL, this_player()->
                        query_skill_bonus( SKILL ) );
          switch( TASKER->perform_task( this_player(), SKILL, difficulty,
            TM_FREE ) ) {
            case AWARD :
              write( "%^YELLOW%^" + ({
                "You discover something that lets your fingers move more "
                  "nimbly.",
                "You find yourself capable of deceiving the eye with greater "
                  "ease than before.",
                "You realise how to deceive the eye more effectively."
              })[ random(3) ] + "%^RESET%^\n" );
            case SUCCEED :
              add_succeeded_mess( ({ "$N $V " + tmp2 + ", managing to stay "
                "unnoticed.\n",
                "" }) );
              break;
            default :
              this_player()->add_succeeded_mess( this_object(), "$N "
                "unsuccessfully tr$y to " + query_verb() + " " + tmp2 +
                " while staying unnoticed.\n", ({ }) );
              break;
          }
        } else {
          this_player()->add_succeeded_mess( this_object(), "$N $V " + tmp2 +
            ".\n", ({ }) );
        }
      }
   }

   /* Put stuff back on that we took off */
   succeeded = ({ });
   failed_rewear = ({ });
   foreach (ob in removed)  {
      if (this_player()->wear_armour(ob))
         failed_rewear += ({ ob });
      else
         succeeded += ({ ob });
   }

   if (sizeof(succeeded) > 0)  {
      // Make sure they drop extra items
      this_player()->force_burden_recalculate();
      tmp1 = query_multiple_short(succeeded, "the");
      write("You wear " + tmp1 + ".\n");
      say(this_player()->the_short() + " wears " + tmp1 + ".\n");
   }

   if (sizeof(failed_rewear) > 0)  {
      write("You cannot put " +
            query_multiple_short(failed_rewear, "the") + " back on.\n");
   }

   if (sizeof(failed) > 0)  {
      write("You cannot remove " + query_multiple_short(failed, "the") +
            ".\n");
   }

   return 1;
}


mixed *query_patterns() {
   return ({ "<indirect:object:me>", (: cmd($1) :) });
} /* query_patterns() */
