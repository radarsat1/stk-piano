/* Piano */

#include "lookup_table.h"

/* PianoDriver.sb */

extern LookupTable noteOffDelayTime;

/* pianoDriverC.sb */
/* Coupling Filter */

extern LookupTable singleStringDecayRate;
extern LookupTable singleStringZero;
extern LookupTable singleStringPole;
extern LookupTable releaseLoopGain;
extern LookupTable detuningHz;
extern LookupTable stiffnessCoefficient;
extern LookupTable strikePosition;
extern LookupTable EQGain;
extern LookupTable EQBandwidthFactor;

/* PianoDriverA */
/* HammerFilter */
extern LookupTable loudPole;
extern LookupTable softPole;
extern LookupTable normalizedVelocity;
extern LookupTable loudGain;
extern LookupTable softGain;


/* Soundboard */
extern LookupTable sustainPedalLevel;
extern LookupTable DryTapAmpT60;
extern double DryTapAmpCurrent;
extern LookupTable DCBa1;


/* pianoDriverB */
/* High Notes */
extern LookupTable secondStageAmpRatio;
extern LookupTable r1_1db;
extern LookupTable r1_2db;
extern LookupTable r2db;
extern LookupTable r3db;
extern LookupTable secondPartialFactor;
extern LookupTable thirdPartialFactor;
extern LookupTable bq4_gEarBalled;
