/******************************************************************************

                                  targets.h

*******************************************************************************/


/******************************** TARGETS *************************************/


/* The target byte (ie. control_ptr->target is divided into 2 parts which 
   consist of the 3 first and last 5 bits.  The first 3 bits are defined in 
   cal_mirror.h   

   The values of the last 5 bits are:                                        */
 
#define TARGET_MASK	0x1f		   // masks 5 least significant bits
                                           // of control_ptr->target
					   
#define SKY	  0
#define MERCURY	  1
#define VENUS	  2
#define EARTH	  3
#define MARS	  4 
#define JUPITER	  5
#define SATURN	  6
#define URANUS	  7 
#define NEPTUNE	  8
#define PLUTO	  9
#define MOON	 10
#define SUN	 11
#define AR	 12   //  Active Region on the SUN
#define STAR	 13

#define BEACON	 20   // test beacon (fixed AZ/EL)
#define SERVICE	 21   // service position (fixed AZ/EL)
#define MANUAL	 22   // manual positioning in AZ/EL

#define LAST_OBJ 30   // 30 object not to be changed
#define UNKNOWN_OBJECT  31
#define DEFAULT_OBJ     UNKNOWN_OBJECT


