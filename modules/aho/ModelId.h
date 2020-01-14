#ifndef IKNOW_AHO_MODELID_H_
#define IKNOW_AHO_MODELID_H_

#ifndef MODELID
#error "MODELID must be defined"
#endif //MODELID

//Model index defaults to 0
#ifndef MODELINDEX
#define MODELINDEX 0
#endif

#define MODELSTRGLUE(x,y) x ## y
#define MODELBUILDNS(x,y) MODELSTRGLUE(x,y)
#define MODELNS MODELBUILDNS(MODELID,MODELINDEX)

#endif //IKNOW_AHO_MODELID_H_
