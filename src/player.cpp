#include <iostream>
#include <functional>
#include <cstdlib>


#include <boost/thread.hpp>
#define TIMER(SECS) \
    if ( m_timer_thread )\
    {\
       m_timer_thread->interrupt ();\
       delete ( m_timer_thread );\
       m_timer_thread = NULL;\
    }\
    m_timer_thread = new boost::thread ( delay_thread , SECS , std::bind ( &player::timer_expired , this ) );\

#include "player.h"

unsigned int Hand_Value ( UberCasino::card_t cards[] )
{
   // given an array of cards, returns the point value
   // this routine is ignorant of the different values
   // for an ace, it is always 1.

   unsigned int total=0;
   for (unsigned int i=0; i< UberCasino::MAX_CARDS_PER_PLAYER;i++)
   {
      if ( cards[i].valid )
      {
         switch ( cards[i].card )
         {
            case ace: total=total+1;cout<<"ace\n";break;
            case two: total=total+2;cout<<"two\n";break;
            case three: total=total+3;cout<<"three\n";break;
            case four: total=total+4;cout<<"four\n";break;
            case five: total=total+5;cout<<"five\n";break;
            case six: total=total+6;cout<<"six\n";break;
            case seven: total=total+7;cout<<"seven\n";break;
            case eight: total=total+8;cout<<"eight\n";break;
            case nine: total=total+9;cout<<"nine\n";break;
            case ten: total=total+10;cout<<"ten\n";break;
            case jack: total=total+10;cout<<"jack\n";break;
            case queen: total=total+10;cout<<"queen\n";break;
            case king: total=total+10;cout<<"king\n";break;
         }
      }
   }
   return total;
}

void delay_thread ( int seconds, std::function <void(void)> callback)
{
  // this routine is created as a posix thread.
  boost::this_thread::sleep_for(boost::chrono::seconds(seconds));
  callback ();
}

void player::lock ()
{
  // if this code is being used as part of an fltk program
  // a lock may be needed.  The fltk lock or something
  // like pthread_mutex() should work fine.
  std::cout << "****************************************" << std::endl;
}

void player::unlock ()
{
  // see comments under the lock () method
}

std::string player::to_string ( player_state_t p )
{
   // turn the player_state enumerant into a string
   std::string retval;
   switch ( p )
   {
      case Init:
         retval = "Init";
         break;
      case StartHand:
         retval = "StartHand";
         break;
      case Playing:
         retval = "Playing";
         break;
      case EndHand:
         retval = "EndHand";
         break;
   }
   return retval;
}

void player::manage_state ()
{
   // determine if we have a state transition
   bool transition = false;
   player_state_t next_state = m_player_state;
   switch ( m_player_state )
   {
     case Init:
         {
            // a dealer is starting a game
            if ( m_Dealer_recv )
            {
                transition = true;
            }
            // the player has entered a string
            if ( m_user_event )
            {
               // you must enter in something that does not
               // exception.  like a number
               m_dealer_idx = std::stoi ( m_user_event_string );
               if ( m_dealer_idx < m_dealer_list.size () )
               {
                  transition = true;
                  next_state = StartHand;
               }
            }
         }
         break;
     case StartHand:
         {
             if ( m_timer_event )
             {
                 transition = true;
                 next_state = Init;
             }
             if ( m_Game_recv_idx )
             {
                 transition = true;
                 next_state = Playing;
             }
         }
         break;
     case Playing:
         {
             if ( m_Game_recv  && m_G.gstate == end_hand )
             {
std::cout << "I did recieve the dealers cards being dealt " << std::endl;
                 // the dealers cards are dealt
                 transition = true;
                 next_state = EndHand; 
             }
             if ( m_Game_recv_idx )
             {
                 transition = true;
                 next_state = Playing;
             }
         }
         break;
     case EndHand:
         {
             if ( m_timer_event )
             {
                 transition = true;
                 next_state = StartHand;
             }
         }
         break;
   }

   if ( m_player_state != next_state )
   {
#ifdef DEBUG_STATES
      std::cout << "State change from " << to_string (m_player_state)
                << " to " << to_string ( next_state ) << std::endl;
#endif
   }
   // if there is a transition, then we have to run the exit 
   // and entrance processing
   if (transition)
   {
      // on exit
      switch (m_player_state)
      {
         case Init:
         {
#ifdef DEBUG_STATES
           std::cout << "Init: Exit" << std::endl;
#endif
           // Wait 30 seconds for the dealer to act
           // if he does not act, then he has not accepted us into the game
           TIMER(30);
         }
         break;
         case StartHand:
         {
#ifdef DEBUG_STATES
           std::cout << "StartHand Exit" << std::endl;
#endif
         }
         break;
         case Playing:
         {
#ifdef DEBUG_STATES
           std::cout << "Playing: Exit" << std::endl;
#endif
         }
         break;
         case EndHand:
         {
#ifdef DEBUG_STATES
           std::cout << "EndHand: Exit" << std::endl;
#endif
           std::cout << "set up all the variables to play another" << std::endl;
         }
         break;
      }

      // on entrance
      switch (next_state)
      {
         case Init:
         {
#ifdef DEBUG_STATES
            std::cout << "Init: Entry" << std::endl;
#endif
            if (m_Dealer_recv)
            {
               m_dealer_list.push_back ( m_D );
            }
            // print the list to stdout
            if (m_dealer_list.size () > 0 )
            {
              for (unsigned int i=0;i<m_dealer_list.size ();i++)
              {
                 std::cout << "Dealer # " << i 
                           << " name " << m_dealer_list[i].name << std::endl;
              }
              std::cout << "Enter the Dealer # to join.." << std::endl;
            }
         }
         break;
         case StartHand:
         {
#ifdef DEBUG_STATES
               std::cout << "Waiting: StartHand" << std::endl;
#endif
               memcpy ( m_P.game_uid, 
                        m_dealer_list[m_dealer_idx].game_uid,
                        sizeof ( m_P.game_uid ) );
               m_P.A = idle;
               // put the game_uuid in a member var a little 
               // easier to find
               memcpy ( &m_current_game_uuid,
                        m_dealer_list[m_dealer_idx].game_uid,
                        sizeof ( m_P.game_uid ) );
               p_io->publish  ( m_P );
         }
         break;
         case Playing:
         {
#ifdef DEBUG_STATES
            std::cout << "Playing: Entry " << std::endl;
#endif
            unsigned int value = Hand_Value ( m_G.p[m_G.active_player].cards );
            std::cout << "The value of my hand is "<< value << std::endl;
            if ( value > 11 )
            {
               std::cout << "I have decided to stand " << std::endl;
               m_P.A = standing;
               TIMER(1);
            }
            else
            {
               std::cout << "I have decided to hit " << std::endl;
               m_P.A = hitting;
            }
            p_io->publish  ( m_P );
         }
         break;
         case EndHand:
         {
#ifdef DEBUG_STATES
            std::cout << "EndHand: Entry " << std::endl;
#endif
std::cout << "the state is " << (int)  m_G.gstate  << std::endl;
            if  ( m_G.gstate == end_hand ) 
            {
              std::cout << "The dealer says end of hand." << std::endl;
              // calculate win or lose
cout<< "Dealer's Hand:\n";
              int dealer_points = Hand_Value ( m_G.dealer_cards ); cout<< "Player's cards: \n";
              int player_points = Hand_Value ( m_G.p[m_G.active_player].cards );
              std::cout << "Dealer has " << dealer_points << " Player has " << player_points << std::endl;
              if ( dealer_points > 21 || ( (player_points > dealer_points) && (player_points < 21) ) || ( (player_points == 21) && (dealer_points != 21) ) )
              {
                 std::cout << "Player Wins" << std::endl;
                 m_balance = m_balance + 10.0;
              }
              else if ( player_points ==  dealer_points )
              {
                 std::cout << "Push" << std::endl;
              }
              else
              {
                 std::cout << "Dealer Wins" << std::endl;
                 m_balance = m_balance - 10.0;
              }

              if (m_balance > 10.0 )
              {
                 // this is just a way to start the next hand
                 TIMER(2);
              }
              else
              {
                 std::cout << "Down to " << m_balance << " exiting from game " << std::endl;
                 exit(0);
              }
            }
         }
         break;
      }

      // make the transition
      m_player_state = next_state;
   }

   // clear all event flags
   m_timer_event   = false;
   m_user_event    = false;
   m_Player_recv   = false;
   m_Game_recv_idx = false;
   m_Game_recv     = false;
   m_Dealer_recv   = false;

}


void player::timer_expired ( )
{
   // this is called by the timer thread callback when the delay has expired
   // note: only one timer can be active at a time
   lock ();
   m_timer_event = true;
   std::cout << "Timer event has been received " << std::endl;
   manage_state ();
   unlock ();
}

void player::external_data (Player P)
{
   // this is called when data is received
   lock ();
   m_P = P;
   m_Player_recv = true;
   std::cout << "Player data has been received" << std::endl;
   manage_state ();
   unlock ();
}

void player::external_data (Dealer D)
{
   // this is called when data is received
   lock ();
   m_D = D;
   m_Dealer_recv = true;
   std::cout << "Dealer data has been received" << std::endl;
   manage_state ();
   unlock ();
}

void player::external_data (Game G)
{
   // only care about games
   // we are in.
   //
   // this routine creates two possible eventsd
   // they are mutually exclusive
   //     m_Game_recv      when the game uids match
   //     m_Game_recv_idx  when the game uids and the idx match
   lock ();

   // first, the game needs to match   
   boost::uuids::uuid t;
   memcpy ( &t, G.game_uid, sizeof ( t ) );
   if (t == m_current_game_uuid)
   {
        m_G = G;  // it is the state of our game, so 
                  // store it
        m_Game_recv = true;
        // now to check if it is our turn
        unsigned int i = G.active_player;
        boost::uuids::uuid active_player_uuid;
        memcpy ( &active_player_uuid,
                 G.p[i].uid,
                 sizeof ( active_player_uuid ) );
        if ( m_my_uid == active_player_uuid )
        {
            // and last, need to be sure we are
            // 'playing' the game
            if ( G.gstate == playing )
            {
               m_Game_recv_idx = true;
               m_Game_recv = false;
            }
        }
   }
   if (m_Game_recv || m_Game_recv_idx )
   {
      if ( m_Game_recv )
      {
        std::cout << "Game (uid matched) received" << std::endl;
      }
      else if ( m_Game_recv_idx )
      {
        std::cout << "Game (uid matched and idx ) received" << std::endl;
      }
      manage_state ();
   }
   else
      // single player, this is a problem.  otherwise it is OK
      // std::cout << "Game was ignored " << std::endl;

   unlock ();
}

void player::user_input (std::string I)
{
   // this is called when the user types in input
   // from the console.  any / all input is accepted
   lock ();
   m_user_event_string = I;
   m_user_event = true;
   std::cout << "User input received" << std::endl;
   manage_state ();
   unlock ();
}

void player::setName (std::string Name )
{
   strncpy ( m_P.name, Name.c_str(), sizeof (m_P.name) - 1 );
}

player::player ()
{
   // member variables
   m_player_state = Init;
   m_balance = 1000.0;
   m_P.balance = m_balance;
   m_dealer_list.clear ();
   m_timer_thread = NULL;
   // member objects
   p_io = new dds_io<Player,PlayerSeq,PlayerTypeSupport_var,PlayerTypeSupport,PlayerDataWriter_var,
                     PlayerDataWriter,PlayerDataReader_var,PlayerDataReader>
                ( (char*) "player", true, false );

   d_io = new dds_io<Dealer,DealerSeq,DealerTypeSupport_var,DealerTypeSupport,DealerDataWriter_var,
                     DealerDataWriter,DealerDataReader_var,DealerDataReader>
                ( (char*) "dealer", false, true );

   g_io = new dds_io<Game,GameSeq,GameTypeSupport_var,GameTypeSupport,GameDataWriter_var,
                     GameDataWriter,GameDataReader_var,GameDataReader>
                ( (char*) "game", false, true );

   // event flags
   m_timer_event    = false;
   m_user_event     = false;
   m_Player_recv    = false;
   m_Game_recv      = false;
   m_Game_recv_idx  = false;
   m_Dealer_recv    = false;
}

player::~player ()
{
}
