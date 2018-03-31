#ifndef DEALER_H
#define DEALER_H

#include <string>

#include <boost/uuid/uuid.hpp>            // uuid class

#include "io.h"

class dealer
{
   private:
      // look at the state diagram to understand the states
      enum dealer_state_t {Init,Waiting,WaitingForOthers,Dealing} m_dealer_state;
      std::string m_name;
      void manage_state ();
      bool timer_event;
      bool m_user_event;
      bool m_Player_recv;
      bool m_Game_recv;
      bool m_Dealer_recv;
      // below are used to filter events
      std::string m_user_event_mask;
       
      std::string print_state ( dealer_state_t );

      // these generics are complicated.  check the ctors of dealer for more info
      dds_io<Player,PlayerSeq,PlayerTypeSupport_var,PlayerTypeSupport,PlayerDataWriter_var,
             PlayerDataWriter,PlayerDataReader_var,PlayerDataReader> *p_io;
      dds_io<Dealer,DealerSeq,DealerTypeSupport_var,DealerTypeSupport,DealerDataWriter_var,
             DealerDataWriter,DealerDataReader_var,DealerDataReader> *d_io;
      dds_io<Game,GameSeq,GameTypeSupport_var,GameTypeSupport,GameDataWriter_var,
             GameDataWriter,GameDataReader_var,GameDataReader> *g_io;
      // these are class members that store the published / recieved structs
      Player m_P_pub;
      Player m_P_sub;
      Dealer m_D_pub;
      Dealer m_D_sub;
      Game   m_G_pub;
      Game   m_G_sub;
   public:
      void setName ( std::string );
      // There are 3 possible inputs to the dealer
      //    1. timer expiration
      //    2. external data recieved
      //    3. user input
      void timer_expired ();
      void external_data ( Player );
      void external_data ( Dealer );
      void external_data ( Game G );
      void setuuid ( boost::uuids::uuid uuid );
      void user_input ( std::string );
      dealer ();
      ~dealer ();
};

#endif
