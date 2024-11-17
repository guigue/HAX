#ifndef HUSEC_H
#define HUSEC_H


uint64_t husec_time(void)
{

        /****************************************************************************************

        HAXtime : Example program to get the time stamp in hundred of microseconds (husec)
          since 0:00:00 UTC using the C time library functions.

          Unix time ("Epoch") does not address correctly the leap seconds and,
          although for us is a minor problem (har to think we'll observe at 23:59:59)
          I do prefer to use our "traditional" SST solution: the husecs since
          the beginning of the observing day.

          clock_gettime(): gives the time in seconds since 01-01-1970 00:00:00 UTC

          To get the number of seconds since 00:00:00 we take the modulus (%)
          of the clock_gettime() output and the number of seconds in a day (86400
          in the uniform Unix time standard)

          Then we convert this number to husecs and add the nanoseconds (converted
          to husecs too).

          To check the results we convert the obtained husecs to hh:mm:ss, and compare
          with the time obatined by using time() function (For some reason, time()
          gives the time in BST and not in UTC).

          This solution will work only while the tv_sec (a signed long int) does not
          overflows. This should happen one day on 2038. Unless the library is
          corrected before.

          Guigue - 14-10-2019 T 18:58 BST

        ******************************************************************************************/

       // Preliminar Definitions
       // ---------------------------------

        static long int SEC2HUSEC       = 10000L        ;
        static long int HUSEC2NSEC      = 100000L       ;
        static long int ONEDAY          = 86400L        ;
       // static long int HUSECS2HRS    = 36000000L     ;
       // static long int MIN2HUSEC     = 600000L       ;
       // static long int MIN           = 60L           ;

        uint64_t all;
        time_t sec;
        struct timespec spec;

        clock_gettime(CLOCK_REALTIME, &spec);
        /*  Convertion procedure  */
        sec = spec.tv_sec % ONEDAY;     // get the number of seconds of the present day getting
                                        // the remainder of the division by 86400 (total number of seconds in a day)

        all = (uint64_t) sec * SEC2HUSEC + (uint64_t) spec.tv_nsec / HUSEC2NSEC; // convert seconds to husecs
                                                                                 // convert nanoseconds of the present second to husecs
                                                                                 // and get the total


        /**************************************************************************
        // Printout the results
        printf("Current time: %" PRIu64  " Hundred of Microseconds since 0:00 \n", all);  // husecs of the day
        printf("Current time (UTC)        : %d:%0.2d:%0.2d\n",
                all / HUSECS2HRS,
                (all / MIN2HUSEC) % MIN ,
                ((all % HUSECS2HRS) % MIN2HUSEC) / SEC2HUSEC);  // husecs converted back to hour, min and seconds
        //printf("Current time (BST = UTC-3): %s",ctime(&now));   // verification by using the internal time() function
                                                                // time() is in local standard time (BST) !
        **************************************************************************/

        return all;
}







#endif
