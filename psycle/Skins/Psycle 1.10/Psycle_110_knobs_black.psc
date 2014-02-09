// source coords in bitmap
// x, y, width, height
// numbers must be fixed height and width

// WARNING!!! Use the same name than the .psc file for one of the three bmp files! The loader requires this.
"machinedial_bmp"="Psycle_110_knobs_black.bmp"
"send_return_bmp"="Psycle_110_mixer_skin_black.bmp"
"master_bmp"="Psycle_110_master_skin.bmp"

//Font used for the parameters and mixer windows.
"params_text_font_name"="Tahoma"
"params_text_font_point"=dword:00000050
"params_text_font_flags"=dword:00000000
"params_text_font_bold_name"="Tahoma"
"params_text_font_bold_point"=dword:00000050
"params_text_font_bold_flags"=dword:00000001

// Note: Colours are written in the form "00BGR" (i.e. opposite of RGB)
"master_text_backcolour"=dword:00000000
"master_text_forecolour"=dword:00FFFFFF
"master_text_font_name"="Tahoma"
"master_text_font_point"=dword:00000050
"master_text_font_flags"=dword:00000000
// names are rendered on the y axis 15 pixels apart. 
"master_text_names_dest"=427,32,75,12
"master_text_numbers_master_dest"=22,186
// slider dBs are rendered on the x axis 24 pixels apart. height is master_text_names_dest height.
"master_text_numbers_channels_dest"=118,186

// These are the constants used for the graphics. They are not user-modifiable because
// it affects some internal objects and calculations.
// rotating knob graphic
// knob (x,y,w,h) 0,0,28,28
// number of knobs :  64  (middle position is knob number 32)

// Mixer graphic positions and widths are:
// mixer slider (x,y,w,h) 0,0,30,182
// mixer slider knob (x,y,w,h) 0,182,22,10
// mixer vu off (x,y,w,h) 30,0,16,90
// mixer vu on (x,y) 46,0
// mixer switch off (x,y,w,h) 30,90,28,28
// mixer switch on (x,y) 30,118
// mixer check off (x,y,w,h) 30,146,13,13
// mixer check on (x,y) 30,159

// Master graphic positions and widths are:
// background (x,y,w,h) 0,0,516,225
// Vu Left off (x,y,w,h) 516,0,18,159
// Vu Left on (x,y) 534,0
// Vu Right off (x,y) 552,0
// Vu Right on (x,y) 570,0
// slider knob (x,y,w,h) 516,159,22,10
