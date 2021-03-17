

void workTheStateMachine()
{
  switch (state)
  {
    case STATE_GETTING_RTCM:   // GPS should be set to send RTCMs, we just wait till the flow starts or resumes
    
      displayStateGettingRtcm();

      _now = millis();

      if(_now - lastReceivedRTCM_ms > 3000)
      {
        // old data doesn't count, clear the buffer, keep waiting:
        rtcms.clear();
        delay(100);
      }
      else if (!rtcms.isEmpty())
      {
        // fresh RTCM data available, proceed to getting network and Caster connected: 
        state = STATE_CONNECTING_TO_WIFI;
      }
      
      break;
    
    case STATE_CONNECTING_TO_WIFI:   // RTCM flowing, we are trying to connect to the network
    
      displayStateConnectingToWifi();

      if(isWifiConnected() || setupWifiNetwork())
      {
        state = STATE_CONNECTING_TO_CASTER;
        displayWifiConnected();
      }

      break;

    case STATE_CONNECTING_TO_CASTER:   // RTCM flowing, WIFI up, we are trying to get response from the caster
    
      displayStateConnectingToCaster();

      switch(tryConnectToCaster())
      {
        case 0:   // already connected

          failedCasterConnectionAttempts = 0;
          displayCasterIsConnected(true);

          state = STATE_WORKING;
          break;
          
        case 1:   // successful new connection

          failedCasterConnectionAttempts = 0;
          displayCasterIsConnected(true);

          // we just connected, need to clear the buffer (which was possibly overfilled)
          rtcms.clear();

          state = STATE_WORKING;
          break;

        case 10:  // Failed to open socket at casterHost, casterPort (wrong address?)
        case 11:  // Failed to get caster response within 5 seconds (caster busy?)
        case 12:  // Caster responded, but not with expected "200"

          // by this time client.close() has been closed, if that was needed.
          
          displayCasterIsConnected(false);

          failedCasterConnectionAttempts++;

          // we want to try re-connecting again in 10 seconds, but if that fails - wait for 5 minutes and repeat at 5 minutes 4 times.
          // also, total attempts over 1500 per day will get our IP address banned at the Caster. Go to doghouse if over 100 attempts.

          if(failedCasterConnectionAttempts < 2)
          {
            delay(10 * 1000); // first try reconnecting in 10 seconds
          }
          else
          {
            // progressively increase interval between connection attempts:
            delay(failedCasterConnectionAttempts * 5 * 60 *1000);
          }
          
          if(failedCasterConnectionAttempts > maxFailedCasterConnectionAttempts)
          {
            Error("Exceeded max connection attempts. Waiting...");
            int minutesLeft = waitAfterFailedConnectionMinutes;
            while(minutesLeft > 0)
            {
              displayWaitAfterFailedConnection(minutesLeft);
              delay(60 * 1000); // wait one minute
              minutesLeft--;
            }
            // now we are free to go:
            failedCasterConnectionAttempts = 0;
          }
         
          break;
      }

      break;

    case STATE_WORKING:   // RTCM flowing, we are connected to caster and sending

      commLed(false); // blue LED will be blinking when data is sent
      displayWorkDataIsFlowing(false);

      // try sending available bytes to Caster, see what happens:
      switch(tryServe())
      {
        case 0:   // all good, but no data in rtcms queue
          //displayStateWorking(serverBytesSent);

          // Close socket if we don't have new data for 10s
          // RTK2Go will ban your IP address if you abuse it. See http://www.rtk2go.com/how-to-get-your-ip-banned/
          // So let's not leave the socket open/hanging without data
          _now = millis();
          if (_now - lastReceivedRTCM_ms > maxTimeBeforeHangup_ms)
          {
            Error("RTCM timeout. Disconnecting...");
            closeCasterConnection();
            lastWentToBreak = _now;
            state = STATE_BREAK_FOR_NO_RTCM;
          }
          
          break;
        
        case 1:   // had good RTCM data, sent to Caster successfully
          // blink blue LEDs:
          commLed(true);
          displayWorkDataIsFlowing(true);
          displayStateWorking(serverBytesSent);
          break;
        
        case 10:  // client disconnected
          // it could be WiFi problem, so start from checking that:
          state = STATE_CONNECTING_TO_WIFI;
          break;
        
        case 11:  // idle switch up
          closeCasterConnection();
          state = STATE_IDLE_BY_SWITCH;
          break;
      }

      break;

    case STATE_BREAK_FOR_NO_RTCM:   // We didn't have RTCMs for 10 seconds, so we disconnected from caster and are waiting 
      displayStateOnBreakNoRtcm();
      delay(100);

      // hope that no-RTCM condition will be resolved in about a minute:
      if(_now - lastWentToBreak > breakInterval_ms)
      {
        state = STATE_GETTING_RTCM;       
      }
      
      break;

    case STATE_IN_DOGHOUSE:   // We exceeded failed connections 24-hour quota and have to wait to avoid being banned.
      displayStateDoghouse();

      if(state != state_prev)
      {
        closeCasterConnection();
      }

      delay(1000);

      // wait till the oldest connection attempt record is cleared: 
      if(connectionAttempts.available())
      {
        state = STATE_IDLE_BY_SWITCH; // will just check the switch and be out
      }
      break;

    case STATE_IDLE_BY_SWITCH:
      displayStateIdle();

      // switch up prevents sending, closes caster connection.

      if(state != state_prev)
      {
        closeCasterConnection();
      }

      delay(100);

      rtcms.clear();  // consume the RTCM data, don't let it get stale in the buffer
      
      if(!isIdleSwitchOn())
      {
        // once idle switch is released, we start from the beginning, getting RTCM:
        state = STATE_GETTING_RTCM;
      }
      break;

    case STATE_FATAL_ERROR:   // Fatal Error state, we need to drop network connections and blink the lights, beep, do nothing until reboot
      displayFatalError();
      break;

    default:
      Error("Invalid state, should not happen");
      break;
  }
  // end of State Machine
}
