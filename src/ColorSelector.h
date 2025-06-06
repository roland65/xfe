#ifndef COLORSELECTOR_H
#define COLORSELECTOR_H

#include <FXColorList.h>


//
// Color names
//


// Named color
struct FXNamedColor
{
    const FXchar*    name;
    FXColor        color;
};


// Massive color database
const FXNamedColor fxcolornames[] =
{
    { "AliceBlue", FXRGBA(240, 248, 255, 255) },
    { "AntiqueWhite", FXRGBA(250, 235, 215, 255) },
    { "AntiqueWhite1", FXRGBA(255, 239, 219, 255) },
    { "AntiqueWhite2", FXRGBA(238, 223, 204, 255) },
    { "AntiqueWhite3", FXRGBA(205, 192, 176, 255) },
    { "AntiqueWhite4", FXRGBA(139, 131, 120, 255) },
    //{"Aqua", FXRGBA(  0,255,255,255)},
    { "Aquamarine", FXRGBA(127, 255, 212, 255) },
    { "Aquamarine1", FXRGBA(127, 255, 212, 255) },
    { "Aquamarine2", FXRGBA(118, 238, 198, 255) },
    { "Aquamarine3", FXRGBA(102, 205, 170, 255) },
    { "Aquamarine4", FXRGBA(69, 139, 116, 255) },
    { "Azure", FXRGBA(240, 255, 255, 255) },
    { "Azure1", FXRGBA(240, 255, 255, 255) },
    { "Azure2", FXRGBA(224, 238, 238, 255) },
    { "Azure3", FXRGBA(193, 205, 205, 255) },
    { "Azure4", FXRGBA(131, 139, 139, 255) },
    { "Banana", FXRGBA(227, 207, 87, 255) },
    { "Beige", FXRGBA(245, 245, 220, 255) },
    { "Bisque", FXRGBA(255, 228, 196, 255) },
    { "Bisque1", FXRGBA(255, 228, 196, 255) },
    { "Bisque2", FXRGBA(238, 213, 183, 255) },
    { "Bisque3", FXRGBA(205, 183, 158, 255) },
    { "Bisque4", FXRGBA(139, 125, 107, 255) },
    { "Black", FXRGBA(0, 0, 0, 255) },
    { "BlanchedAlmond", FXRGBA(255, 235, 205, 255) },
    { "Blue", FXRGBA(0, 0, 255, 255) },
    { "Blue1", FXRGBA(0, 0, 255, 255) },
    { "Blue2", FXRGBA(0, 0, 238, 255) },
    { "Blue3", FXRGBA(0, 0, 205, 255) },
    { "Blue4", FXRGBA(0, 0, 139, 255) },
    { "BlueViolet", FXRGBA(138, 43, 226, 255) },
    { "Brick", FXRGBA(156, 102, 31, 255) },
    { "Brown", FXRGBA(165, 42, 42, 255) },
    { "Brown1", FXRGBA(255, 64, 64, 255) },
    { "Brown2", FXRGBA(238, 59, 59, 255) },
    { "Brown3", FXRGBA(205, 51, 51, 255) },
    { "Brown4", FXRGBA(139, 35, 35, 255) },
    { "Burlywood", FXRGBA(222, 184, 135, 255) },
    { "Burlywood1", FXRGBA(255, 211, 155, 255) },
    { "Burlywood2", FXRGBA(238, 197, 145, 255) },
    { "Burlywood3", FXRGBA(205, 170, 125, 255) },
    { "Burlywood4", FXRGBA(139, 115, 85, 255) },
    { "BurnedSienna", FXRGBA(138, 54, 15, 255) },
    { "BurnedUmber", FXRGBA(138, 51, 36, 255) },
    { "CadetBlue", FXRGBA(95, 158, 160, 255) },
    { "CadetBlue1", FXRGBA(152, 245, 255, 255) },
    { "CadetBlue2", FXRGBA(142, 229, 238, 255) },
    { "CadetBlue3", FXRGBA(122, 197, 205, 255) },
    { "CadetBlue4", FXRGBA(83, 134, 139, 255) },
    { "CadmiumOrange", FXRGBA(255, 97, 3, 255) },
    { "CadmiumRed", FXRGBA(227, 23, 13, 255) },
    { "CadmiumYellow", FXRGBA(255, 153, 18, 255) },
    { "Carrot", FXRGBA(237, 145, 33, 255) },
    { "Chartreuse", FXRGBA(127, 255, 0, 255) },
    { "Chartreuse1", FXRGBA(127, 255, 0, 255) },
    { "Chartreuse2", FXRGBA(118, 238, 0, 255) },
    { "Chartreuse3", FXRGBA(102, 205, 0, 255) },
    { "Chartreuse4", FXRGBA(69, 139, 0, 255) },
    { "Chocolate", FXRGBA(210, 105, 30, 255) },
    { "Chocolate1", FXRGBA(255, 127, 36, 255) },
    { "Chocolate2", FXRGBA(238, 118, 33, 255) },
    { "Chocolate3", FXRGBA(205, 102, 29, 255) },
    { "Chocolate4", FXRGBA(139, 69, 19, 255) },
    { "Cobalt", FXRGBA(61, 89, 171, 255) },
    { "CobaltGreen", FXRGBA(61, 145, 64, 255) },
    { "ColdGrey", FXRGBA(128, 138, 135, 255) },
    { "Coral", FXRGBA(255, 127, 80, 255) },
    { "Coral1", FXRGBA(255, 114, 86, 255) },
    { "Coral2", FXRGBA(238, 106, 80, 255) },
    { "Coral3", FXRGBA(205, 91, 69, 255) },
    { "Coral4", FXRGBA(139, 62, 47, 255) },
    { "CornflowerBlue", FXRGBA(100, 149, 237, 255) },
    { "Cornsilk", FXRGBA(255, 248, 220, 255) },
    { "Cornsilk1", FXRGBA(255, 248, 220, 255) },
    { "Cornsilk2", FXRGBA(238, 232, 205, 255) },
    { "Cornsilk3", FXRGBA(205, 200, 177, 255) },
    { "Cornsilk4", FXRGBA(139, 136, 120, 255) },
    { "Crimson", FXRGBA(220, 20, 60, 255) },
    { "Cyan", FXRGBA(0, 255, 255, 255) },
    { "Cyan1", FXRGBA(0, 255, 255, 255) },
    { "Cyan2", FXRGBA(0, 238, 238, 255) },
    { "Cyan3", FXRGBA(0, 205, 205, 255) },
    { "Cyan4", FXRGBA(0, 139, 139, 255) },
    { "DarkBlue", FXRGBA(0, 0, 139, 255) },
    { "DarkCyan", FXRGBA(0, 139, 139, 255) },
    { "DarkGoldenrod", FXRGBA(184, 134, 11, 255) },
    { "DarkGoldenrod1", FXRGBA(255, 185, 15, 255) },
    { "DarkGoldenrod2", FXRGBA(238, 173, 14, 255) },
    { "DarkGoldenrod3", FXRGBA(205, 149, 12, 255) },
    { "DarkGoldenrod4", FXRGBA(139, 101, 8, 255) },
    { "DarkGray", FXRGBA(169, 169, 169, 255) },
    { "DarkGreen", FXRGBA(0, 100, 0, 255) },
    { "DarkGrey", FXRGBA(169, 169, 169, 255) },
    { "DarkKhaki", FXRGBA(189, 183, 107, 255) },
    { "DarkMagenta", FXRGBA(139, 0, 139, 255) },
    { "DarkOliveGreen", FXRGBA(85, 107, 47, 255) },
    { "DarkOliveGreen1", FXRGBA(202, 255, 112, 255) },
    { "DarkOliveGreen2", FXRGBA(188, 238, 104, 255) },
    { "DarkOliveGreen3", FXRGBA(162, 205, 90, 255) },
    { "DarkOliveGreen4", FXRGBA(110, 139, 61, 255) },
    { "DarkOrange", FXRGBA(255, 140, 0, 255) },
    { "DarkOrange1", FXRGBA(255, 127, 0, 255) },
    { "DarkOrange2", FXRGBA(238, 118, 0, 255) },
    { "DarkOrange3", FXRGBA(205, 102, 0, 255) },
    { "DarkOrange4", FXRGBA(139, 69, 0, 255) },
    { "DarkOrchid", FXRGBA(153, 50, 204, 255) },
    { "DarkOrchid1", FXRGBA(191, 62, 255, 255) },
    { "DarkOrchid2", FXRGBA(178, 58, 238, 255) },
    { "DarkOrchid3", FXRGBA(154, 50, 205, 255) },
    { "DarkOrchid4", FXRGBA(104, 34, 139, 255) },
    { "DarkRed", FXRGBA(139, 0, 0, 255) },
    { "DarkSalmon", FXRGBA(233, 150, 122, 255) },
    { "DarkSeaGreen", FXRGBA(143, 188, 143, 255) },
    { "DarkSeaGreen1", FXRGBA(193, 255, 193, 255) },
    { "DarkSeaGreen2", FXRGBA(180, 238, 180, 255) },
    { "DarkSeaGreen3", FXRGBA(155, 205, 155, 255) },
    { "DarkSeaGreen4", FXRGBA(105, 139, 105, 255) },
    { "DarkSlateBlue", FXRGBA(72, 61, 139, 255) },
    { "DarkSlateGray", FXRGBA(47, 79, 79, 255) },
    { "DarkSlateGray1", FXRGBA(151, 255, 255, 255) },
    { "DarkSlateGray2", FXRGBA(141, 238, 238, 255) },
    { "DarkSlateGray3", FXRGBA(121, 205, 205, 255) },
    { "DarkSlateGray4", FXRGBA(82, 139, 139, 255) },
    { "DarkSlateGrey", FXRGBA(47, 79, 79, 255) },
    { "DarkTurquoise", FXRGBA(0, 206, 209, 255) },
    { "DarkViolet", FXRGBA(148, 0, 211, 255) },
    { "DeepPink", FXRGBA(255, 20, 147, 255) },
    { "DeepPink1", FXRGBA(255, 20, 147, 255) },
    { "DeepPink2", FXRGBA(238, 18, 137, 255) },
    { "DeepPink3", FXRGBA(205, 16, 118, 255) },
    { "DeepPink4", FXRGBA(139, 10, 80, 255) },
    { "DeepSkyBlue", FXRGBA(0, 191, 255, 255) },
    { "DeepSkyBlue1", FXRGBA(0, 191, 255, 255) },
    { "DeepSkyBlue2", FXRGBA(0, 178, 238, 255) },
    { "DeepSkyBlue3", FXRGBA(0, 154, 205, 255) },
    { "DeepSkyBlue4", FXRGBA(0, 104, 139, 255) },
    { "DimGray", FXRGBA(105, 105, 105, 255) },
    { "DimGrey", FXRGBA(105, 105, 105, 255) },
    { "DodgerBlue", FXRGBA(30, 144, 255, 255) },
    { "DodgerBlue1", FXRGBA(30, 144, 255, 255) },
    { "DodgerBlue2", FXRGBA(28, 134, 238, 255) },
    { "DodgerBlue3", FXRGBA(24, 116, 205, 255) },
    { "DodgerBlue4", FXRGBA(16, 78, 139, 255) },
    { "Eggshell", FXRGBA(252, 230, 201, 255) },
    { "EmeraldGreen", FXRGBA(0, 201, 87, 255) },
    { "Firebrick", FXRGBA(178, 34, 34, 255) },
    { "Firebrick1", FXRGBA(255, 48, 48, 255) },
    { "Firebrick2", FXRGBA(238, 44, 44, 255) },
    { "Firebrick3", FXRGBA(205, 38, 38, 255) },
    { "Firebrick4", FXRGBA(139, 26, 26, 255) },
    { "FloralWhite", FXRGBA(255, 250, 240, 255) },
    { "ForestGreen", FXRGBA(34, 139, 34, 255) },
    //{"Fuchsia",              FXRGBA(255,  0,255,255)},
    { "Gainsboro", FXRGBA(220, 220, 220, 255) },
    { "GhostWhite", FXRGBA(248, 248, 255, 255) },
    { "Gold", FXRGBA(255, 215, 0, 255) },
    { "Gold1", FXRGBA(255, 215, 0, 255) },
    { "Gold2", FXRGBA(238, 201, 0, 255) },
    { "Gold3", FXRGBA(205, 173, 0, 255) },
    { "Gold4", FXRGBA(139, 117, 0, 255) },
    { "Goldenrod", FXRGBA(218, 165, 32, 255) },
    { "Goldenrod1", FXRGBA(255, 193, 37, 255) },
    { "Goldenrod2", FXRGBA(238, 180, 34, 255) },
    { "Goldenrod3", FXRGBA(205, 155, 29, 255) },
    { "Goldenrod4", FXRGBA(139, 105, 20, 255) },
    { "Gray", FXRGBA(190, 190, 190, 255) },
    { "Gray0", FXRGBA(0, 0, 0, 255) },
    { "Gray1", FXRGBA(3, 3, 3, 255) },
    { "Gray10", FXRGBA(26, 26, 26, 255) },
    { "Gray100", FXRGBA(255, 255, 255, 255) },
    { "Gray11", FXRGBA(28, 28, 28, 255) },
    { "Gray12", FXRGBA(31, 31, 31, 255) },
    { "Gray13", FXRGBA(33, 33, 33, 255) },
    { "Gray14", FXRGBA(36, 36, 36, 255) },
    { "Gray15", FXRGBA(38, 38, 38, 255) },
    { "Gray16", FXRGBA(41, 41, 41, 255) },
    { "Gray17", FXRGBA(43, 43, 43, 255) },
    { "Gray18", FXRGBA(46, 46, 46, 255) },
    { "Gray19", FXRGBA(48, 48, 48, 255) },
    { "Gray2", FXRGBA(5, 5, 5, 255) },
    { "Gray20", FXRGBA(51, 51, 51, 255) },
    { "Gray21", FXRGBA(54, 54, 54, 255) },
    { "Gray22", FXRGBA(56, 56, 56, 255) },
    { "Gray23", FXRGBA(59, 59, 59, 255) },
    { "Gray24", FXRGBA(61, 61, 61, 255) },
    { "Gray25", FXRGBA(64, 64, 64, 255) },
    { "Gray26", FXRGBA(66, 66, 66, 255) },
    { "Gray27", FXRGBA(69, 69, 69, 255) },
    { "Gray28", FXRGBA(71, 71, 71, 255) },
    { "Gray29", FXRGBA(74, 74, 74, 255) },
    { "Gray3", FXRGBA(8, 8, 8, 255) },
    { "Gray30", FXRGBA(77, 77, 77, 255) },
    { "Gray31", FXRGBA(79, 79, 79, 255) },
    { "Gray32", FXRGBA(82, 82, 82, 255) },
    { "Gray33", FXRGBA(84, 84, 84, 255) },
    { "Gray34", FXRGBA(87, 87, 87, 255) },
    { "Gray35", FXRGBA(89, 89, 89, 255) },
    { "Gray36", FXRGBA(92, 92, 92, 255) },
    { "Gray37", FXRGBA(94, 94, 94, 255) },
    { "Gray38", FXRGBA(97, 97, 97, 255) },
    { "Gray39", FXRGBA(99, 99, 99, 255) },
    { "Gray4", FXRGBA(10, 10, 10, 255) },
    { "Gray40", FXRGBA(102, 102, 102, 255) },
    { "Gray41", FXRGBA(105, 105, 105, 255) },
    { "Gray42", FXRGBA(107, 107, 107, 255) },
    { "Gray43", FXRGBA(110, 110, 110, 255) },
    { "Gray44", FXRGBA(112, 112, 112, 255) },
    { "Gray45", FXRGBA(115, 115, 115, 255) },
    { "Gray46", FXRGBA(117, 117, 117, 255) },
    { "Gray47", FXRGBA(120, 120, 120, 255) },
    { "Gray48", FXRGBA(122, 122, 122, 255) },
    { "Gray49", FXRGBA(125, 125, 125, 255) },
    { "Gray5", FXRGBA(13, 13, 13, 255) },
    { "Gray50", FXRGBA(127, 127, 127, 255) },
    { "Gray51", FXRGBA(130, 130, 130, 255) },
    { "Gray52", FXRGBA(133, 133, 133, 255) },
    { "Gray53", FXRGBA(135, 135, 135, 255) },
    { "Gray54", FXRGBA(138, 138, 138, 255) },
    { "Gray55", FXRGBA(140, 140, 140, 255) },
    { "Gray56", FXRGBA(143, 143, 143, 255) },
    { "Gray57", FXRGBA(145, 145, 145, 255) },
    { "Gray58", FXRGBA(148, 148, 148, 255) },
    { "Gray59", FXRGBA(150, 150, 150, 255) },
    { "Gray6", FXRGBA(15, 15, 15, 255) },
    { "Gray60", FXRGBA(153, 153, 153, 255) },
    { "Gray61", FXRGBA(156, 156, 156, 255) },
    { "Gray62", FXRGBA(158, 158, 158, 255) },
    { "Gray63", FXRGBA(161, 161, 161, 255) },
    { "Gray64", FXRGBA(163, 163, 163, 255) },
    { "Gray65", FXRGBA(166, 166, 166, 255) },
    { "Gray66", FXRGBA(168, 168, 168, 255) },
    { "Gray67", FXRGBA(171, 171, 171, 255) },
    { "Gray68", FXRGBA(173, 173, 173, 255) },
    { "Gray69", FXRGBA(176, 176, 176, 255) },
    { "Gray7", FXRGBA(18, 18, 18, 255) },
    { "Gray70", FXRGBA(179, 179, 179, 255) },
    { "Gray71", FXRGBA(181, 181, 181, 255) },
    { "Gray72", FXRGBA(184, 184, 184, 255) },
    { "Gray73", FXRGBA(186, 186, 186, 255) },
    { "Gray74", FXRGBA(189, 189, 189, 255) },
    { "Gray75", FXRGBA(191, 191, 191, 255) },
    { "Gray76", FXRGBA(194, 194, 194, 255) },
    { "Gray77", FXRGBA(196, 196, 196, 255) },
    { "Gray78", FXRGBA(199, 199, 199, 255) },
    { "Gray79", FXRGBA(201, 201, 201, 255) },
    { "Gray8", FXRGBA(20, 20, 20, 255) },
    { "Gray80", FXRGBA(204, 204, 204, 255) },
    { "Gray81", FXRGBA(207, 207, 207, 255) },
    { "Gray82", FXRGBA(209, 209, 209, 255) },
    { "Gray83", FXRGBA(212, 212, 212, 255) },
    { "Gray84", FXRGBA(214, 214, 214, 255) },
    { "Gray85", FXRGBA(217, 217, 217, 255) },
    { "Gray86", FXRGBA(219, 219, 219, 255) },
    { "Gray87", FXRGBA(222, 222, 222, 255) },
    { "Gray88", FXRGBA(224, 224, 224, 255) },
    { "Gray89", FXRGBA(227, 227, 227, 255) },
    { "Gray9", FXRGBA(23, 23, 23, 255) },
    { "Gray90", FXRGBA(229, 229, 229, 255) },
    { "Gray91", FXRGBA(232, 232, 232, 255) },
    { "Gray92", FXRGBA(235, 235, 235, 255) },
    { "Gray93", FXRGBA(237, 237, 237, 255) },
    { "Gray94", FXRGBA(240, 240, 240, 255) },
    { "Gray95", FXRGBA(242, 242, 242, 255) },
    { "Gray96", FXRGBA(245, 245, 245, 255) },
    { "Gray97", FXRGBA(247, 247, 247, 255) },
    { "Gray98", FXRGBA(250, 250, 250, 255) },
    { "Gray99", FXRGBA(252, 252, 252, 255) },
    { "Green", FXRGBA(0, 255, 0, 255) },
    { "Green1", FXRGBA(0, 255, 0, 255) },
    { "Green2", FXRGBA(0, 238, 0, 255) },
    { "Green3", FXRGBA(0, 205, 0, 255) },
    { "Green4", FXRGBA(0, 139, 0, 255) },
    { "GreenYellow", FXRGBA(173, 255, 47, 255) },
    { "Grey", FXRGBA(190, 190, 190, 255) },
    { "Grey0", FXRGBA(0, 0, 0, 255) },
    { "Grey1", FXRGBA(3, 3, 3, 255) },
    { "Grey10", FXRGBA(26, 26, 26, 255) },
    { "Grey100", FXRGBA(255, 255, 255, 255) },
    { "Grey11", FXRGBA(28, 28, 28, 255) },
    { "Grey12", FXRGBA(31, 31, 31, 255) },
    { "Grey13", FXRGBA(33, 33, 33, 255) },
    { "Grey14", FXRGBA(36, 36, 36, 255) },
    { "Grey15", FXRGBA(38, 38, 38, 255) },
    { "Grey16", FXRGBA(41, 41, 41, 255) },
    { "Grey17", FXRGBA(43, 43, 43, 255) },
    { "Grey18", FXRGBA(46, 46, 46, 255) },
    { "Grey19", FXRGBA(48, 48, 48, 255) },
    { "Grey2", FXRGBA(5, 5, 5, 255) },
    { "Grey20", FXRGBA(51, 51, 51, 255) },
    { "Grey21", FXRGBA(54, 54, 54, 255) },
    { "Grey22", FXRGBA(56, 56, 56, 255) },
    { "Grey23", FXRGBA(59, 59, 59, 255) },
    { "Grey24", FXRGBA(61, 61, 61, 255) },
    { "Grey25", FXRGBA(64, 64, 64, 255) },
    { "Grey26", FXRGBA(66, 66, 66, 255) },
    { "Grey27", FXRGBA(69, 69, 69, 255) },
    { "Grey28", FXRGBA(71, 71, 71, 255) },
    { "Grey29", FXRGBA(74, 74, 74, 255) },
    { "Grey3", FXRGBA(8, 8, 8, 255) },
    { "Grey30", FXRGBA(77, 77, 77, 255) },
    { "Grey31", FXRGBA(79, 79, 79, 255) },
    { "Grey32", FXRGBA(82, 82, 82, 255) },
    { "Grey33", FXRGBA(84, 84, 84, 255) },
    { "Grey34", FXRGBA(87, 87, 87, 255) },
    { "Grey35", FXRGBA(89, 89, 89, 255) },
    { "Grey36", FXRGBA(92, 92, 92, 255) },
    { "Grey37", FXRGBA(94, 94, 94, 255) },
    { "Grey38", FXRGBA(97, 97, 97, 255) },
    { "Grey39", FXRGBA(99, 99, 99, 255) },
    { "Grey4", FXRGBA(10, 10, 10, 255) },
    { "Grey40", FXRGBA(102, 102, 102, 255) },
    { "Grey41", FXRGBA(105, 105, 105, 255) },
    { "Grey42", FXRGBA(107, 107, 107, 255) },
    { "Grey43", FXRGBA(110, 110, 110, 255) },
    { "Grey44", FXRGBA(112, 112, 112, 255) },
    { "Grey45", FXRGBA(115, 115, 115, 255) },
    { "Grey46", FXRGBA(117, 117, 117, 255) },
    { "Grey47", FXRGBA(120, 120, 120, 255) },
    { "Grey48", FXRGBA(122, 122, 122, 255) },
    { "Grey49", FXRGBA(125, 125, 125, 255) },
    { "Grey5", FXRGBA(13, 13, 13, 255) },
    { "Grey50", FXRGBA(127, 127, 127, 255) },
    { "Grey51", FXRGBA(130, 130, 130, 255) },
    { "Grey52", FXRGBA(133, 133, 133, 255) },
    { "Grey53", FXRGBA(135, 135, 135, 255) },
    { "Grey54", FXRGBA(138, 138, 138, 255) },
    { "Grey55", FXRGBA(140, 140, 140, 255) },
    { "Grey56", FXRGBA(143, 143, 143, 255) },
    { "Grey57", FXRGBA(145, 145, 145, 255) },
    { "Grey58", FXRGBA(148, 148, 148, 255) },
    { "Grey59", FXRGBA(150, 150, 150, 255) },
    { "Grey6", FXRGBA(15, 15, 15, 255) },
    { "Grey60", FXRGBA(153, 153, 153, 255) },
    { "Grey61", FXRGBA(156, 156, 156, 255) },
    { "Grey62", FXRGBA(158, 158, 158, 255) },
    { "Grey63", FXRGBA(161, 161, 161, 255) },
    { "Grey64", FXRGBA(163, 163, 163, 255) },
    { "Grey65", FXRGBA(166, 166, 166, 255) },
    { "Grey66", FXRGBA(168, 168, 168, 255) },
    { "Grey67", FXRGBA(171, 171, 171, 255) },
    { "Grey68", FXRGBA(173, 173, 173, 255) },
    { "Grey69", FXRGBA(176, 176, 176, 255) },
    { "Grey7", FXRGBA(18, 18, 18, 255) },
    { "Grey70", FXRGBA(179, 179, 179, 255) },
    { "Grey71", FXRGBA(181, 181, 181, 255) },
    { "Grey72", FXRGBA(184, 184, 184, 255) },
    { "Grey73", FXRGBA(186, 186, 186, 255) },
    { "Grey74", FXRGBA(189, 189, 189, 255) },
    { "Grey75", FXRGBA(191, 191, 191, 255) },
    { "Grey76", FXRGBA(194, 194, 194, 255) },
    { "Grey77", FXRGBA(196, 196, 196, 255) },
    { "Grey78", FXRGBA(199, 199, 199, 255) },
    { "Grey79", FXRGBA(201, 201, 201, 255) },
    { "Grey8", FXRGBA(20, 20, 20, 255) },
    { "Grey80", FXRGBA(204, 204, 204, 255) },
    { "Grey81", FXRGBA(207, 207, 207, 255) },
    { "Grey82", FXRGBA(209, 209, 209, 255) },
    { "Grey83", FXRGBA(212, 212, 212, 255) },
    { "Grey84", FXRGBA(214, 214, 214, 255) },
    { "Grey85", FXRGBA(217, 217, 217, 255) },
    { "Grey86", FXRGBA(219, 219, 219, 255) },
    { "Grey87", FXRGBA(222, 222, 222, 255) },
    { "Grey88", FXRGBA(224, 224, 224, 255) },
    { "Grey89", FXRGBA(227, 227, 227, 255) },
    { "Grey9", FXRGBA(23, 23, 23, 255) },
    { "Grey90", FXRGBA(229, 229, 229, 255) },
    { "Grey91", FXRGBA(232, 232, 232, 255) },
    { "Grey92", FXRGBA(235, 235, 235, 255) },
    { "Grey93", FXRGBA(237, 237, 237, 255) },
    { "Grey94", FXRGBA(240, 240, 240, 255) },
    { "Grey95", FXRGBA(242, 242, 242, 255) },
    { "Grey96", FXRGBA(245, 245, 245, 255) },
    { "Grey97", FXRGBA(247, 247, 247, 255) },
    { "Grey98", FXRGBA(250, 250, 250, 255) },
    { "Grey99", FXRGBA(252, 252, 252, 255) },
    { "Honeydew", FXRGBA(240, 255, 240, 255) },
    { "Honeydew1", FXRGBA(240, 255, 240, 255) },
    { "Honeydew2", FXRGBA(224, 238, 224, 255) },
    { "Honeydew3", FXRGBA(193, 205, 193, 255) },
    { "Honeydew4", FXRGBA(131, 139, 131, 255) },
    { "HotPink", FXRGBA(255, 105, 180, 255) },
    { "HotPink1", FXRGBA(255, 110, 180, 255) },
    { "HotPink2", FXRGBA(238, 106, 167, 255) },
    { "HotPink3", FXRGBA(205, 96, 144, 255) },
    { "HotPink4", FXRGBA(139, 58, 98, 255) },
    { "IndianRed", FXRGBA(205, 92, 92, 255) },
    { "IndianRed1", FXRGBA(255, 106, 106, 255) },
    { "IndianRed2", FXRGBA(238, 99, 99, 255) },
    { "IndianRed3", FXRGBA(205, 85, 85, 255) },
    { "IndianRed4", FXRGBA(139, 58, 58, 255) },
    { "Indigo", FXRGBA(75, 0, 130, 255) },
    { "Ivory", FXRGBA(255, 255, 240, 255) },
    { "Ivory1", FXRGBA(255, 255, 240, 255) },
    { "Ivory2", FXRGBA(238, 238, 224, 255) },
    { "Ivory3", FXRGBA(205, 205, 193, 255) },
    { "Ivory4", FXRGBA(139, 139, 131, 255) },
    { "Khaki", FXRGBA(240, 230, 140, 255) },
    { "Khaki1", FXRGBA(255, 246, 143, 255) },
    { "Khaki2", FXRGBA(238, 230, 133, 255) },
    { "Khaki3", FXRGBA(205, 198, 115, 255) },
    { "Khaki4", FXRGBA(139, 134, 78, 255) },
    { "Lavender", FXRGBA(230, 230, 250, 255) },
    { "LavenderBlush", FXRGBA(255, 240, 245, 255) },
    { "LavenderBlush1", FXRGBA(255, 240, 245, 255) },
    { "LavenderBlush2", FXRGBA(238, 224, 229, 255) },
    { "LavenderBlush3", FXRGBA(205, 193, 197, 255) },
    { "LavenderBlush4", FXRGBA(139, 131, 134, 255) },
    { "LawnGreen", FXRGBA(124, 252, 0, 255) },
    { "LemonChiffon", FXRGBA(255, 250, 205, 255) },
    { "LemonChiffon1", FXRGBA(255, 250, 205, 255) },
    { "LemonChiffon2", FXRGBA(238, 233, 191, 255) },
    { "LemonChiffon3", FXRGBA(205, 201, 165, 255) },
    { "LemonChiffon4", FXRGBA(139, 137, 112, 255) },
    { "LightBlue", FXRGBA(173, 216, 230, 255) },
    { "LightBlue1", FXRGBA(191, 239, 255, 255) },
    { "LightBlue2", FXRGBA(178, 223, 238, 255) },
    { "LightBlue3", FXRGBA(154, 192, 205, 255) },
    { "LightBlue4", FXRGBA(104, 131, 139, 255) },
    { "LightCoral", FXRGBA(240, 128, 128, 255) },
    { "LightCyan", FXRGBA(224, 255, 255, 255) },
    { "LightCyan1", FXRGBA(224, 255, 255, 255) },
    { "LightCyan2", FXRGBA(209, 238, 238, 255) },
    { "LightCyan3", FXRGBA(180, 205, 205, 255) },
    { "LightCyan4", FXRGBA(122, 139, 139, 255) },
    { "LightGoldenrod", FXRGBA(238, 221, 130, 255) },
    { "LightGoldenrod1", FXRGBA(255, 236, 139, 255) },
    { "LightGoldenrod2", FXRGBA(238, 220, 130, 255) },
    { "LightGoldenrod3", FXRGBA(205, 190, 112, 255) },
    { "LightGoldenrod4", FXRGBA(139, 129, 76, 255) },
    { "LightGoldenrodYellow", FXRGBA(250, 250, 210, 255) },
    { "LightGray", FXRGBA(211, 211, 211, 255) },
    { "LightGreen", FXRGBA(144, 238, 144, 255) },
    { "LightGrey", FXRGBA(211, 211, 211, 255) },
    { "LightPink", FXRGBA(255, 182, 193, 255) },
    { "LightPink1", FXRGBA(255, 174, 185, 255) },
    { "LightPink2", FXRGBA(238, 162, 173, 255) },
    { "LightPink3", FXRGBA(205, 140, 149, 255) },
    { "LightPink4", FXRGBA(139, 95, 101, 255) },
    { "LightSalmon", FXRGBA(255, 160, 122, 255) },
    { "LightSalmon1", FXRGBA(255, 160, 122, 255) },
    { "LightSalmon2", FXRGBA(238, 149, 114, 255) },
    { "LightSalmon3", FXRGBA(205, 129, 98, 255) },
    { "LightSalmon4", FXRGBA(139, 87, 66, 255) },
    { "LightSeaGreen", FXRGBA(32, 178, 170, 255) },
    { "LightSkyBlue", FXRGBA(135, 206, 250, 255) },
    { "LightSkyBlue1", FXRGBA(176, 226, 255, 255) },
    { "LightSkyBlue2", FXRGBA(164, 211, 238, 255) },
    { "LightSkyBlue3", FXRGBA(141, 182, 205, 255) },
    { "LightSkyBlue4", FXRGBA(96, 123, 139, 255) },
    { "LightSlateBlue", FXRGBA(132, 112, 255, 255) },
    { "LightSlateGray", FXRGBA(119, 136, 153, 255) },
    { "LightSlateGrey", FXRGBA(119, 136, 153, 255) },
    { "LightSteelBlue", FXRGBA(176, 196, 222, 255) },
    { "LightSteelBlue1", FXRGBA(202, 225, 255, 255) },
    { "LightSteelBlue2", FXRGBA(188, 210, 238, 255) },
    { "LightSteelBlue3", FXRGBA(162, 181, 205, 255) },
    { "LightSteelBlue4", FXRGBA(110, 123, 139, 255) },
    { "LightYellow", FXRGBA(255, 255, 224, 255) },
    { "LightYellow1", FXRGBA(255, 255, 224, 255) },
    { "LightYellow2", FXRGBA(238, 238, 209, 255) },
    { "LightYellow3", FXRGBA(205, 205, 180, 255) },
    { "LightYellow4", FXRGBA(139, 139, 122, 255) },
    //{"Lime", FXRGBA(  0,255,  0,255)},
    { "LimeGreen", FXRGBA(50, 205, 50, 255) },
    { "Linen", FXRGBA(250, 240, 230, 255) },
    { "Magenta", FXRGBA(255, 0, 255, 255) },
    { "Magenta1", FXRGBA(255, 0, 255, 255) },
    { "Magenta2", FXRGBA(238, 0, 238, 255) },
    { "Magenta3", FXRGBA(205, 0, 205, 255) },
    { "Magenta4", FXRGBA(139, 0, 139, 255) },
    { "Maroon", FXRGBA(176, 48, 96, 255) },
    { "Maroon1", FXRGBA(255, 52, 179, 255) },
    { "Maroon2", FXRGBA(238, 48, 167, 255) },
    { "Maroon3", FXRGBA(205, 41, 144, 255) },
    { "Maroon4", FXRGBA(139, 28, 98, 255) },
    { "MediumAquamarine", FXRGBA(102, 205, 170, 255) },
    { "MediumBlue", FXRGBA(0, 0, 205, 255) },
    { "MediumOrchid", FXRGBA(186, 85, 211, 255) },
    { "MediumOrchid1", FXRGBA(224, 102, 255, 255) },
    { "MediumOrchid2", FXRGBA(209, 95, 238, 255) },
    { "MediumOrchid3", FXRGBA(180, 82, 205, 255) },
    { "MediumOrchid4", FXRGBA(122, 55, 139, 255) },
    { "MediumPurple", FXRGBA(147, 112, 219, 255) },
    { "MediumPurple1", FXRGBA(171, 130, 255, 255) },
    { "MediumPurple2", FXRGBA(159, 121, 238, 255) },
    { "MediumPurple3", FXRGBA(137, 104, 205, 255) },
    { "MediumPurple4", FXRGBA(93, 71, 139, 255) },
    { "MediumSeaGreen", FXRGBA(60, 179, 113, 255) },
    { "MediumSlateBlue", FXRGBA(123, 104, 238, 255) },
    { "MediumSpringGreen", FXRGBA(0, 250, 154, 255) },
    { "MediumTurquoise", FXRGBA(72, 209, 204, 255) },
    { "MediumVioletRed", FXRGBA(199, 21, 133, 255) },
    { "MidnightBlue", FXRGBA(25, 25, 112, 255) },
    { "MintCream", FXRGBA(245, 255, 250, 255) },
    { "MistyRose", FXRGBA(255, 228, 225, 255) },
    { "MistyRose1", FXRGBA(255, 228, 225, 255) },
    { "MistyRose2", FXRGBA(238, 213, 210, 255) },
    { "MistyRose3", FXRGBA(205, 183, 181, 255) },
    { "MistyRose4", FXRGBA(139, 125, 123, 255) },
    { "Moccasin", FXRGBA(255, 228, 181, 255) },
    { "NavajoWhite", FXRGBA(255, 222, 173, 255) },
    { "NavajoWhite1", FXRGBA(255, 222, 173, 255) },
    { "NavajoWhite2", FXRGBA(238, 207, 161, 255) },
    { "NavajoWhite3", FXRGBA(205, 179, 139, 255) },
    { "NavajoWhite4", FXRGBA(139, 121, 94, 255) },
    { "Navy", FXRGBA(0, 0, 128, 255) },
    { "NavyBlue", FXRGBA(0, 0, 128, 255) },
    { "None", FXRGBA(0, 0, 0, 0) },             // Transparent
    { "OldLace", FXRGBA(253, 245, 230, 255) },
    { "Olive", FXRGBA(128, 128, 0, 255) },
    { "OliveDrab", FXRGBA(107, 142, 35, 255) },
    { "OliveDrab1", FXRGBA(192, 255, 62, 255) },
    { "OliveDrab2", FXRGBA(179, 238, 58, 255) },
    { "OliveDrab3", FXRGBA(154, 205, 50, 255) },
    { "OliveDrab4", FXRGBA(105, 139, 34, 255) },
    { "Orange", FXRGBA(255, 165, 0, 255) },
    { "Orange1", FXRGBA(255, 165, 0, 255) },
    { "Orange2", FXRGBA(238, 154, 0, 255) },
    { "Orange3", FXRGBA(205, 133, 0, 255) },
    { "Orange4", FXRGBA(139, 90, 0, 255) },
    { "OrangeRed", FXRGBA(255, 69, 0, 255) },
    { "OrangeRed1", FXRGBA(255, 69, 0, 255) },
    { "OrangeRed2", FXRGBA(238, 64, 0, 255) },
    { "OrangeRed3", FXRGBA(205, 55, 0, 255) },
    { "OrangeRed4", FXRGBA(139, 37, 0, 255) },
    { "Orchid", FXRGBA(218, 112, 214, 255) },
    { "Orchid1", FXRGBA(255, 131, 250, 255) },
    { "Orchid2", FXRGBA(238, 122, 233, 255) },
    { "Orchid3", FXRGBA(205, 105, 201, 255) },
    { "Orchid4", FXRGBA(139, 71, 137, 255) },
    { "PaleGoldenrod", FXRGBA(238, 232, 170, 255) },
    { "PaleGreen", FXRGBA(152, 251, 152, 255) },
    { "PaleGreen1", FXRGBA(154, 255, 154, 255) },
    { "PaleGreen2", FXRGBA(144, 238, 144, 255) },
    { "PaleGreen3", FXRGBA(124, 205, 124, 255) },
    { "PaleGreen4", FXRGBA(84, 139, 84, 255) },
    { "PaleTurquoise", FXRGBA(175, 238, 238, 255) },
    { "PaleTurquoise1", FXRGBA(187, 255, 255, 255) },
    { "PaleTurquoise2", FXRGBA(174, 238, 238, 255) },
    { "PaleTurquoise3", FXRGBA(150, 205, 205, 255) },
    { "PaleTurquoise4", FXRGBA(102, 139, 139, 255) },
    { "PaleVioletRed", FXRGBA(219, 112, 147, 255) },
    { "PaleVioletRed1", FXRGBA(255, 130, 171, 255) },
    { "PaleVioletRed2", FXRGBA(238, 121, 159, 255) },
    { "PaleVioletRed3", FXRGBA(205, 104, 137, 255) },
    { "PaleVioletRed4", FXRGBA(139, 71, 93, 255) },
    { "PapayaWhip", FXRGBA(255, 239, 213, 255) },
    { "PeachPuff", FXRGBA(255, 218, 185, 255) },
    { "PeachPuff1", FXRGBA(255, 218, 185, 255) },
    { "PeachPuff2", FXRGBA(238, 203, 173, 255) },
    { "PeachPuff3", FXRGBA(205, 175, 149, 255) },
    { "PeachPuff4", FXRGBA(139, 119, 101, 255) },
    { "Peru", FXRGBA(205, 133, 63, 255) },
    { "Pink", FXRGBA(255, 192, 203, 255) },
    { "Pink1", FXRGBA(255, 181, 197, 255) },
    { "Pink2", FXRGBA(238, 169, 184, 255) },
    { "Pink3", FXRGBA(205, 145, 158, 255) },
    { "Pink4", FXRGBA(139, 99, 108, 255) },
    { "Plum", FXRGBA(221, 160, 221, 255) },
    { "Plum1", FXRGBA(255, 187, 255, 255) },
    { "Plum2", FXRGBA(238, 174, 238, 255) },
    { "Plum3", FXRGBA(205, 150, 205, 255) },
    { "Plum4", FXRGBA(139, 102, 139, 255) },
    { "PowderBlue", FXRGBA(176, 224, 230, 255) },
    { "Purple", FXRGBA(160, 32, 240, 255) },
    { "Purple1", FXRGBA(155, 48, 255, 255) },
    { "Purple2", FXRGBA(145, 44, 238, 255) },
    { "Purple3", FXRGBA(125, 38, 205, 255) },
    { "Purple4", FXRGBA(85, 26, 139, 255) },
    { "Raspberry", FXRGBA(135, 38, 87, 255) },
    { "Red", FXRGBA(255, 0, 0, 255) },
    { "Red1", FXRGBA(255, 0, 0, 255) },
    { "Red2", FXRGBA(238, 0, 0, 255) },
    { "Red3", FXRGBA(205, 0, 0, 255) },
    { "Red4", FXRGBA(139, 0, 0, 255) },
    { "RosyBrown", FXRGBA(188, 143, 143, 255) },
    { "RosyBrown1", FXRGBA(255, 193, 193, 255) },
    { "RosyBrown2", FXRGBA(238, 180, 180, 255) },
    { "RosyBrown3", FXRGBA(205, 155, 155, 255) },
    { "RosyBrown4", FXRGBA(139, 105, 105, 255) },
    { "RoyalBlue", FXRGBA(65, 105, 225, 255) },
    { "RoyalBlue1", FXRGBA(72, 118, 255, 255) },
    { "RoyalBlue2", FXRGBA(67, 110, 238, 255) },
    { "RoyalBlue3", FXRGBA(58, 95, 205, 255) },
    { "RoyalBlue4", FXRGBA(39, 64, 139, 255) },
    { "SaddleBrown", FXRGBA(139, 69, 19, 255) },
    { "Salmon", FXRGBA(250, 128, 114, 255) },
    { "Salmon1", FXRGBA(255, 140, 105, 255) },
    { "Salmon2", FXRGBA(238, 130, 98, 255) },
    { "Salmon3", FXRGBA(205, 112, 84, 255) },
    { "Salmon4", FXRGBA(139, 76, 57, 255) },
    { "SandyBrown", FXRGBA(244, 164, 96, 255) },
    { "SeaGreen", FXRGBA(46, 139, 87, 255) },
    { "SeaGreen1", FXRGBA(84, 255, 159, 255) },
    { "SeaGreen2", FXRGBA(78, 238, 148, 255) },
    { "SeaGreen3", FXRGBA(67, 205, 128, 255) },
    { "SeaGreen4", FXRGBA(46, 139, 87, 255) },
    { "Seashell", FXRGBA(255, 245, 238, 255) },
    { "Seashell1", FXRGBA(255, 245, 238, 255) },
    { "Seashell2", FXRGBA(238, 229, 222, 255) },
    { "Seashell3", FXRGBA(205, 197, 191, 255) },
    { "Seashell4", FXRGBA(139, 134, 130, 255) },
    { "Sepia", FXRGBA(94, 38, 18, 255) },
    { "Sienna", FXRGBA(160, 82, 45, 255) },
    { "Sienna1", FXRGBA(255, 130, 71, 255) },
    { "Sienna2", FXRGBA(238, 121, 66, 255) },
    { "Sienna3", FXRGBA(205, 104, 57, 255) },
    { "Sienna4", FXRGBA(139, 71, 38, 255) },
    { "Silver", FXRGBA(192, 192, 192, 255) },
    { "SkyBlue", FXRGBA(135, 206, 235, 255) },
    { "SkyBlue1", FXRGBA(135, 206, 255, 255) },
    { "SkyBlue2", FXRGBA(126, 192, 238, 255) },
    { "SkyBlue3", FXRGBA(108, 166, 205, 255) },
    { "SkyBlue4", FXRGBA(74, 112, 139, 255) },
    { "SlateBlue", FXRGBA(106, 90, 205, 255) },
    { "SlateBlue1", FXRGBA(131, 111, 255, 255) },
    { "SlateBlue2", FXRGBA(122, 103, 238, 255) },
    { "SlateBlue3", FXRGBA(105, 89, 205, 255) },
    { "SlateBlue4", FXRGBA(71, 60, 139, 255) },
    { "SlateGray", FXRGBA(112, 128, 144, 255) },
    { "SlateGray1", FXRGBA(198, 226, 255, 255) },
    { "SlateGray2", FXRGBA(185, 211, 238, 255) },
    { "SlateGray3", FXRGBA(159, 182, 205, 255) },
    { "SlateGray4", FXRGBA(108, 123, 139, 255) },
    { "SlateGrey", FXRGBA(112, 128, 144, 255) },
    { "Snow", FXRGBA(255, 250, 250, 255) },
    { "Snow1", FXRGBA(255, 250, 250, 255) },
    { "Snow2", FXRGBA(238, 233, 233, 255) },
    { "Snow3", FXRGBA(205, 201, 201, 255) },
    { "Snow4", FXRGBA(139, 137, 137, 255) },
    { "SpringGreen", FXRGBA(0, 255, 127, 255) },
    { "SpringGreen1", FXRGBA(0, 255, 127, 255) },
    { "SpringGreen2", FXRGBA(0, 238, 118, 255) },
    { "SpringGreen3", FXRGBA(0, 205, 102, 255) },
    { "SpringGreen4", FXRGBA(0, 139, 69, 255) },
    { "SteelBlue", FXRGBA(70, 130, 180, 255) },
    { "SteelBlue1", FXRGBA(99, 184, 255, 255) },
    { "SteelBlue2", FXRGBA(92, 172, 238, 255) },
    { "SteelBlue3", FXRGBA(79, 148, 205, 255) },
    { "SteelBlue4", FXRGBA(54, 100, 139, 255) },
    { "Tan", FXRGBA(210, 180, 140, 255) },
    { "Tan1", FXRGBA(255, 165, 79, 255) },
    { "Tan2", FXRGBA(238, 154, 73, 255) },
    { "Tan3", FXRGBA(205, 133, 63, 255) },
    { "Tan4", FXRGBA(139, 90, 43, 255) },
    { "Teal", FXRGBA(0, 128, 128, 255) },
    { "Thistle", FXRGBA(216, 191, 216, 255) },
    { "Thistle1", FXRGBA(255, 225, 255, 255) },
    { "Thistle2", FXRGBA(238, 210, 238, 255) },
    { "Thistle3", FXRGBA(205, 181, 205, 255) },
    { "Thistle4", FXRGBA(139, 123, 139, 255) },
    { "Tomato", FXRGBA(255, 99, 71, 255) },
    { "Tomato1", FXRGBA(255, 99, 71, 255) },
    { "Tomato2", FXRGBA(238, 92, 66, 255) },
    { "Tomato3", FXRGBA(205, 79, 57, 255) },
    { "Tomato4", FXRGBA(139, 54, 38, 255) },
    { "Turquoise", FXRGBA(64, 224, 208, 255) },
    { "Turquoise1", FXRGBA(0, 245, 255, 255) },
    { "Turquoise2", FXRGBA(0, 229, 238, 255) },
    { "Turquoise3", FXRGBA(0, 197, 205, 255) },
    { "Turquoise4", FXRGBA(0, 134, 139, 255) },
    { "Ultramarine", FXRGBA(0, 42, 143, 255) },
    { "Violet", FXRGBA(238, 130, 238, 255) },
    { "VioletRed", FXRGBA(208, 32, 144, 255) },
    { "VioletRed1", FXRGBA(255, 62, 150, 255) },
    { "VioletRed2", FXRGBA(238, 58, 140, 255) },
    { "VioletRed3", FXRGBA(205, 50, 120, 255) },
    { "VioletRed4", FXRGBA(139, 34, 82, 255) },
    { "WarmGrey", FXRGBA(128, 128, 105, 255) },
    { "Wheat", FXRGBA(245, 222, 179, 255) },
    { "Wheat1", FXRGBA(255, 231, 186, 255) },
    { "Wheat2", FXRGBA(238, 216, 174, 255) },
    { "Wheat3", FXRGBA(205, 186, 150, 255) },
    { "Wheat4", FXRGBA(139, 126, 102, 255) },
    { "White", FXRGBA(255, 255, 255, 255) },
    { "WhiteSmoke", FXRGBA(245, 245, 245, 255) },
    { "Yellow", FXRGBA(255, 255, 0, 255) },
    { "Yellow1", FXRGBA(255, 255, 0, 255) },
    { "Yellow2", FXRGBA(238, 238, 0, 255) },
    { "Yellow3", FXRGBA(205, 205, 0, 255) },
    { "Yellow4", FXRGBA(139, 139, 0, 255) },
    { "YellowGreen", FXRGBA(154, 205, 50, 255) },
};


// Number of color names
const FXuint fxnumcolornames = ARRAYNUMBER(fxcolornames);


//
// ColorWell class
//


// Color Well Styles
enum
{
    _COLORWELL_OPAQUEONLY    = 0x00008000,           // Colors must be opaque
    _COLORWELL_SOURCEONLY    = 0x00010000,           // ColorWell is never a target
    _COLORWELL_NORMAL        = JUSTIFY_NORMAL
};


/**
 * A color well is a widget which controls color settings.
 * Colors may be dragged and dropped from one color well to another.
 * A double-click inside a color well will bring up the standard
 * color dialog panel to edit the color well's color.
 * Colors may be also pasted by name using middle-mouse click into/out of
 * color wells from/to other selection-capable applications; for example,
 * you can highlight the word `red' and paste it into a color well.
 * While the color value is being changed, the color well sends a SEL_CHANGED
 * to its target; at the end of the change, it sends a SEL_COMMAND.
 * The message data represents the current color value, of the type FXColor.
 */
class FXAPI ColorWell : public FXFrame
{
    FXDECLARE(ColorWell)

protected:
    FXColor wellColor[2];                               // Pixel value of RGBA over black and white
    FXColor rgba = FXRGB(0, 0, 0);                      // Color with RGB and Alpha
    FXString tip;                                       // Tooltip value
    FXString help;                                      // Help value

protected:
    ColorWell();
    static FXColor rgbaoverblack(FXColor clr);
    static FXColor rgbaoverwhite(FXColor clr);

private:
    ColorWell(const ColorWell&);
    ColorWell &operator=(const ColorWell&);

public:
    long onPaint(FXObject*, FXSelector, void*);
    long onLeftBtnPress(FXObject*, FXSelector, void*);
    long onLeftBtnRelease(FXObject*, FXSelector, void*);
    long onMiddleBtnPress(FXObject*, FXSelector, void*);
    long onMiddleBtnRelease(FXObject*, FXSelector, void*);
    long onKeyPress(FXObject*, FXSelector, void*);
    long onKeyRelease(FXObject*, FXSelector, void*);
    long onUngrabbed(FXObject*, FXSelector, void*);
    long onMotion(FXObject*, FXSelector, void*);
    long onBeginDrag(FXObject*, FXSelector, void*);
    long onEndDrag(FXObject*, FXSelector, void*);
    long onDragged(FXObject*, FXSelector, void*);
    long onFocusIn(FXObject*, FXSelector, void*);
    long onFocusOut(FXObject*, FXSelector, void*);
    long onDNDEnter(FXObject*, FXSelector, void*);
    long onDNDLeave(FXObject*, FXSelector, void*);
    long onDNDMotion(FXObject*, FXSelector, void*);
    long onDNDDrop(FXObject*, FXSelector, void*);
    long onDNDRequest(FXObject*, FXSelector, void*);
    long onSelectionLost(FXObject*, FXSelector, void*);
    long onSelectionGained(FXObject*, FXSelector, void*);
    long onSelectionRequest(FXObject*, FXSelector, void*);
    long onClicked(FXObject*, FXSelector, void*);
    long onDoubleClicked(FXObject*, FXSelector, void*);
    long onCmdSetValue(FXObject*, FXSelector, void*);
    long onCmdSetIntValue(FXObject*, FXSelector, void*);
    long onCmdGetIntValue(FXObject*, FXSelector, void*);
    long onCmdColorWell(FXObject*, FXSelector, void*);
    long onChgColorWell(FXObject*, FXSelector, void*);
    long onCmdSetHelp(FXObject*, FXSelector, void*);
    long onCmdGetHelp(FXObject*, FXSelector, void*);
    long onCmdSetTip(FXObject*, FXSelector, void*);
    long onCmdGetTip(FXObject*, FXSelector, void*);
    long onQueryHelp(FXObject*, FXSelector, void*);
    long onQueryTip(FXObject*, FXSelector, void*);

public:
    enum
    {
        ID_COLORDIALOG=FXFrame::ID_LAST,
        ID_LAST
    };

public:

    // Construct color well with initial color clr
    ColorWell(FXComposite* p, FXColor clr = 0, FXObject* tgt = NULL, FXSelector sel = 0, FXuint opts = COLORWELL_NORMAL,
              FXint x = 0, FXint y = 0, FXint w = 0, FXint h = 0, FXint pl = DEFAULT_PAD, FXint pr = DEFAULT_PAD,
              FXint pt = DEFAULT_PAD, FXint pb = DEFAULT_PAD);

    // Create server-side resources
    virtual void create();

    // Detach server-side resources
    virtual void detach();

    // Return default width
    virtual FXint getDefaultWidth();

    // Return default height
    virtual FXint getDefaultHeight();

    // Returns true because a color well can receive focus
    virtual bool canFocus() const;

    // Move the focus to this window
    virtual void setFocus();

    // Remove the focus from this window
    virtual void killFocus();

    // Set color
    void setRGBA(FXColor clr, FXbool notify = FALSE);

    // Get color
    FXColor getRGBA() const
    {
        return rgba;
    }

    // Set status line help text for this color well
    void setHelpText(const FXString& text)
    {
        help = text;
    }

    // Get status line help text for this color well
    const FXString& getHelpText() const
    {
        return help;
    }

    // Set tool tip message for this color well
    void setTipText(const FXString& text)
    {
        tip = text;
    }

    // Get tool tip message for this color well
    const FXString& getTipText() const
    {
        return tip;
    }

    // Return true if only opaque colors allowed
    FXbool isOpaqueOnly() const;

    // Change opaque only mode
    void setOpaqueOnly(FXbool opaque);

    // Save color well to a stream
    virtual void save(FXStream& store) const;

    // Load color well from a stream
    virtual void load(FXStream& store);

    // Destructor
    virtual ~ColorWell();
};


//
// ColorSelector class
//


// Color selection widget
class FXAPI ColorSelector : public FXPacker
{
    FXDECLARE(ColorSelector)

protected:
    FXTabBook* panels = NULL;                                   // Color panes
    ColorWell* well = NULL;                                     // Main well
    FXColorList* list = NULL;                                   // List of color names
    FXColorRing* wheel = NULL;                                  // Color ring
    FXSlider* rgbaslider[4] = { NULL };                         // RGBA sliders
    FXTextField* rgbatext[4] = { NULL };                        // RGBA text fields
    FXSlider* hsvaslider[4] = { NULL };                         // HSVA sliders
    FXTextField* hsvatext[4] = { NULL };                        // HSVA text fields
    FXSlider* cmyslider[4] = { NULL };                          // CMY sliders
    FXTextField* cmytext[4] = { NULL };                         // CMY text fields
    ColorWell* colorwells[24] = { NULL };                       // Custom color wells
    FXButton* accept = NULL;                                    // Accept button
    FXButton* cancel = NULL;                                    // Cancel button
    FXfloat rgba[4];                                            // Accurate RGBA color
    FXfloat hsva[4];                                            // Accurate HSVA color

protected:
    static const FXchar* wellname[24];                          // Well names

protected:
    ColorSelector()
    {
    }
    void updateWell();

private:
    ColorSelector(const ColorSelector&);
    ColorSelector &operator=(const ColorSelector&);

public:
    long onCmdWell(FXObject*, FXSelector, void*);
    long onChgWell(FXObject*, FXSelector, void*);
    long onCmdRGBSlider(FXObject*, FXSelector, void*);
    long onUpdRGBSlider(FXObject*, FXSelector, void*);
    long onCmdRGBText(FXObject*, FXSelector, void*);
    long onUpdRGBText(FXObject*, FXSelector, void*);
    long onCmdHSVSlider(FXObject*, FXSelector, void*);
    long onUpdHSVSlider(FXObject*, FXSelector, void*);
    long onCmdHSVText(FXObject*, FXSelector, void*);
    long onUpdHSVText(FXObject*, FXSelector, void*);
    long onCmdCMYSlider(FXObject*, FXSelector, void*);
    long onUpdCMYSlider(FXObject*, FXSelector, void*);
    long onCmdCMYText(FXObject*, FXSelector, void*);
    long onUpdCMYText(FXObject*, FXSelector, void*);
    long onCmdList(FXObject*, FXSelector, void*);
    long onCmdCustomWell(FXObject*, FXSelector, void*);
    long onChgCustomWell(FXObject*, FXSelector, void*);
    long onCmdSetValue(FXObject*, FXSelector, void*);
    long onCmdActivePane(FXObject*, FXSelector, void*);
    long onCmdAlphaSlider(FXObject*, FXSelector, void*);
    long onUpdAlphaSlider(FXObject*, FXSelector, void*);
    long onCmdAlphaText(FXObject*, FXSelector, void*);
    long onUpdAlphaText(FXObject*, FXSelector, void*);
    long onUpdAlphaLabel(FXObject*, FXSelector, void*);
    long onCmdWheel(FXObject*, FXSelector, void*);
    long onUpdWheel(FXObject*, FXSelector, void*);
    long onCmdColorPick(FXObject*, FXSelector, void*);

public:
    enum
    {
        ID_CUSTOM_FIRST =FXPacker::ID_LAST,
        ID_CUSTOM_LAST    =ID_CUSTOM_FIRST + 24,
        ID_RGB_RED_SLIDER,
        ID_RGB_GREEN_SLIDER,
        ID_RGB_BLUE_SLIDER,
        ID_RGB_RED_TEXT,
        ID_RGB_GREEN_TEXT,
        ID_RGB_BLUE_TEXT,
        ID_HSV_HUE_SLIDER,
        ID_HSV_SATURATION_SLIDER,
        ID_HSV_VALUE_SLIDER,
        ID_HSV_HUE_TEXT,
        ID_HSV_SATURATION_TEXT,
        ID_HSV_VALUE_TEXT,
        ID_CMY_CYAN_SLIDER,
        ID_CMY_MAGENTA_SLIDER,
        ID_CMY_YELLOW_SLIDER,
        ID_CMY_CYAN_TEXT,
        ID_CMY_MAGENTA_TEXT,
        ID_CMY_YELLOW_TEXT,
        ID_DIAL_WHEEL,
        ID_COLOR_LIST,
        ID_WELL_CHANGED,
        ID_ACTIVEPANE,
        ID_ALPHA_SLIDER,
        ID_ALPHA_TEXT,
        ID_ALPHA_LABEL,
        ID_COLORPICK,
        ID_LAST
    };

public:

    // Construct a new ColorSelector
    ColorSelector(FXComposite* p, FXObject* tgt = NULL, FXSelector sel = 0, FXuint opts = 0,
                  FXint x = 0, FXint y = 0, FXint w = 0, FXint h = 0);

    // Create the ColorSelector
    virtual void create();

    // Return a pointer to the "Accept" button
    FXButton*acceptButton() const
    {
        return accept;
    }

    // Return a pointer to the "Cancel" button
    FXButton*cancelButton() const
    {
        return cancel;
    }

    // Set the selected color
    void setRGBA(FXColor clr);

    // Get the selected color
    FXColor getRGBA() const;

    // Return true if only opaque colors allowed
    FXbool isOpaqueOnly() const;

    // Change opaque only mode
    void setOpaqueOnly(FXbool opaque);

    // Save to a stream
    virtual void save(FXStream& store) const;

    // Load from a stream
    virtual void load(FXStream& store);

    // Destructor
    virtual ~ColorSelector();
};


//
// ColorDialog class
//


/**
 * The Color dialog is a standard dialog panel used to edit colors.
 * Colors can be edited via RGB (Red, Green, Blue additive color model),
 * via HSV (Hue, Saturation, Value color modal), via CMY (Cyan, Magenta,
 * Yellow subtractive color model), or by name.
 * Commonly used colors can be dragged into a number of small color wells
 * to be used repeatedly; colors dropped into the small color wells are
 * automatically saved into the registry for future use.
 */
class FXAPI ColorDialog : public FXDialogBox
{
    FXDECLARE(ColorDialog)

protected:
    ColorSelector* colorbox = NULL;

protected:
    ColorDialog()
    {
    }

private:
    ColorDialog(const ColorDialog&);
    ColorDialog &operator=(const ColorDialog&);

public:
    long onChgColor(FXObject*, FXSelector, void*);
    long onCmdColor(FXObject*, FXSelector, void*);

public:
    enum
    {
        ID_COLORSELECTOR=FXDialogBox::ID_LAST,
        ID_LAST
    };

public:

    // Construct color dialog
    ColorDialog(FXWindow* owner, const FXString& name, FXuint opts = 0, FXint x = 0, FXint y = 0, FXint w = 0,
                FXint h = 0);

    // Set the color
    void setRGBA(FXColor clr);

    // Get the color
    FXColor getRGBA() const;

    // Return true if only opaque colors allowed
    FXbool isOpaqueOnly() const;

    // Change opaque only mode
    void setOpaqueOnly(FXbool forceopaque);

    // Save dialog to a stream
    virtual void save(FXStream& store) const;

    // Load dialog from a stream
    virtual void load(FXStream& store);

    // Destructor
    virtual ~ColorDialog();
};

#endif
