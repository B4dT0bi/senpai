
// includes

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "attack.hpp"
#include "bit.hpp"
#include "common.hpp"
#include "eval.hpp"
#include "hash.hpp"
#include "libmy.hpp"
#include "math.hpp"
#include "pawn.hpp"
#include "pos.hpp"
#include "score.hpp"
#include "var.hpp"

// constants

const int  Pawn_Table_Bit  { 12 };
const int  Pawn_Table_Size { 1 << Pawn_Table_Bit };
const int  Pawn_Table_Mask { Pawn_Table_Size - 1 };

const int  Scale { 100 }; // units per cp

// types

class Score_Pair {

private :

   int64 p_vec;

public :

   Score_Pair ();
   explicit Score_Pair (int sc);
   Score_Pair (int mg, int eg);

   void operator += (Score_Pair sp);
   void operator -= (Score_Pair sp);

   friend Score_Pair operator + (Score_Pair sp);
   friend Score_Pair operator - (Score_Pair sp);

   friend Score_Pair operator + (Score_Pair s0, Score_Pair s1);
   friend Score_Pair operator - (Score_Pair s0, Score_Pair s1);

   friend Score_Pair operator * (Score_Pair weight, int n);
   friend Score_Pair operator * (Score_Pair weight, double x);

   int mg () const;
   int eg () const;

private :

   static Score_Pair make (int64 vec);
};

struct Pawn_Info {
   Key key;
   Score_Pair score[Side_Size];
   Bit passed[Side_Size];
   Bit strong[Side_Size];
   float centre_file, centre_rank;
};

// "constants"

Score_Pair W[] = { // 10000 units = 1 pawn
   Score_Pair(9049, 12537),
   Score_Pair(29594, 34965),
   Score_Pair(32125, 34190),
   Score_Pair(44928, 61719),
   Score_Pair(109411, 111079),
   Score_Pair(0, 0),
   Score_Pair(2401, 5495),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(-2397, -2337),
   Score_Pair(-1202, -984),
   Score_Pair(-1163, -2026),
   Score_Pair(-1328, -2398),
   Score_Pair(-2594, -1854),
   Score_Pair(-2325, -649),
   Score_Pair(-2419, -2331),
   Score_Pair(-2543, -2677),
   Score_Pair(-1454, -1489),
   Score_Pair(-1470, -366),
   Score_Pair(-856, -2582),
   Score_Pair(-756, -3333),
   Score_Pair(-895, 1005),
   Score_Pair(-513, 1150),
   Score_Pair(654, -790),
   Score_Pair(452, -1523),
   Score_Pair(2233, 4345),
   Score_Pair(3282, 5158),
   Score_Pair(3339, 3007),
   Score_Pair(3618, 2163),
   Score_Pair(847, 2012),
   Score_Pair(2658, 4050),
   Score_Pair(1707, 2395),
   Score_Pair(2132, 2390),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(-510, -2179),
   Score_Pair(138, -73),
   Score_Pair(1331, 561),
   Score_Pair(1695, 1449),
   Score_Pair(-665, -1382),
   Score_Pair(-230, -589),
   Score_Pair(311, -312),
   Score_Pair(195, 373),
   Score_Pair(-2164, -1906),
   Score_Pair(-2316, -1857),
   Score_Pair(-982, -330),
   Score_Pair(-1122, -338),
   Score_Pair(407, 73),
   Score_Pair(666, 1116),
   Score_Pair(2047, 1886),
   Score_Pair(1738, 2914),
   Score_Pair(-1223, -509),
   Score_Pair(-606, 4),
   Score_Pair(414, 1079),
   Score_Pair(1040, 1402),
   Score_Pair(-2352, -722),
   Score_Pair(-381, -141),
   Score_Pair(1644, 1273),
   Score_Pair(1572, 1205),
   Score_Pair(-2431, -1084),
   Score_Pair(-1439, -719),
   Score_Pair(993, 474),
   Score_Pair(1522, 1057),
   Score_Pair(-2075, -2610),
   Score_Pair(-811, -620),
   Score_Pair(361, 1558),
   Score_Pair(640, 1223),
   Score_Pair(-1433, -4592),
   Score_Pair(-1148, -2541),
   Score_Pair(696, -1434),
   Score_Pair(554, -88),
   Score_Pair(-3293, -3598),
   Score_Pair(-1999, -2566),
   Score_Pair(-498, -187),
   Score_Pair(53, 625),
   Score_Pair(-1110, -1805),
   Score_Pair(615, 1104),
   Score_Pair(2127, 2903),
   Score_Pair(2068, 3194),
   Score_Pair(-1570, -1179),
   Score_Pair(976, 1533),
   Score_Pair(2911, 3036),
   Score_Pair(3451, 3251),
   Score_Pair(-2391, -1132),
   Score_Pair(290, 969),
   Score_Pair(2153, 2728),
   Score_Pair(3897, 2646),
   Score_Pair(-4069, -2417),
   Score_Pair(-755, 318),
   Score_Pair(1993, 1214),
   Score_Pair(2518, 1847),
   Score_Pair(-4256, -3217),
   Score_Pair(-1721, -1336),
   Score_Pair(531, 1072),
   Score_Pair(681, 951),
   Score_Pair(-2205, -2216),
   Score_Pair(-316, -79),
   Score_Pair(575, 1282),
   Score_Pair(211, 1237),
   Score_Pair(-4225, -2535),
   Score_Pair(-2652, -1299),
   Score_Pair(-534, -736),
   Score_Pair(-486, -559),
   Score_Pair(-3800, -2644),
   Score_Pair(-2019, -2244),
   Score_Pair(-463, -473),
   Score_Pair(-586, -474),
   Score_Pair(-3789, -3370),
   Score_Pair(-2833, -1876),
   Score_Pair(-1316, -1292),
   Score_Pair(-1926, -789),
   Score_Pair(-1656, -1534),
   Score_Pair(-787, 211),
   Score_Pair(778, 1305),
   Score_Pair(1150, 1366),
   Score_Pair(-1615, -370),
   Score_Pair(-194, 333),
   Score_Pair(373, 1619),
   Score_Pair(1470, 1391),
   Score_Pair(-1494, 1390),
   Score_Pair(1117, 1687),
   Score_Pair(2613, 2700),
   Score_Pair(3361, 2743),
   Score_Pair(-27, 1724),
   Score_Pair(2148, 3002),
   Score_Pair(3807, 3827),
   Score_Pair(4186, 3842),
   Score_Pair(25, -794),
   Score_Pair(735, 800),
   Score_Pair(2187, 2128),
   Score_Pair(1382, 1865),
   Score_Pair(-3402, -3217),
   Score_Pair(-1498, -2246),
   Score_Pair(-356, -1401),
   Score_Pair(537, 510),
   Score_Pair(-2646, -2878),
   Score_Pair(-1460, -1867),
   Score_Pair(313, 1),
   Score_Pair(1175, 1644),
   Score_Pair(-440, -982),
   Score_Pair(850, 483),
   Score_Pair(2079, 2385),
   Score_Pair(2641, 2728),
   Score_Pair(-1308, -480),
   Score_Pair(578, 849),
   Score_Pair(1534, 1850),
   Score_Pair(2657, 2046),
   Score_Pair(-1851, -1342),
   Score_Pair(-547, 1058),
   Score_Pair(1075, 1520),
   Score_Pair(2004, 884),
   Score_Pair(-2335, -1151),
   Score_Pair(1, 583),
   Score_Pair(1792, 1168),
   Score_Pair(2194, 2047),
   Score_Pair(-1546, -569),
   Score_Pair(221, 348),
   Score_Pair(2064, 1881),
   Score_Pair(1843, 1619),
   Score_Pair(-632, -571),
   Score_Pair(445, 478),
   Score_Pair(1267, 1402),
   Score_Pair(1675, 1581),
   Score_Pair(4734, -688),
   Score_Pair(3926, 928),
   Score_Pair(-1467, 110),
   Score_Pair(-3130, -1816),
   Score_Pair(6686, 524),
   Score_Pair(3255, 1096),
   Score_Pair(-2387, 1217),
   Score_Pair(-4713, 526),
   Score_Pair(2341, 158),
   Score_Pair(2166, 536),
   Score_Pair(-2022, 213),
   Score_Pair(-3550, -152),
   Score_Pair(-777, -1106),
   Score_Pair(-384, -233),
   Score_Pair(-1039, -403),
   Score_Pair(-2352, -1232),
   Score_Pair(-1090, -1399),
   Score_Pair(113, 269),
   Score_Pair(-714, 33),
   Score_Pair(-1074, -468),
   Score_Pair(-459, -447),
   Score_Pair(1321, 1830),
   Score_Pair(1186, 1903),
   Score_Pair(226, 896),
   Score_Pair(-987, -1465),
   Score_Pair(1529, 1780),
   Score_Pair(1840, 2224),
   Score_Pair(934, 1291),
   Score_Pair(-4136, -5749),
   Score_Pair(-111, -186),
   Score_Pair(284, 255),
   Score_Pair(432, 484),
   Score_Pair(993, 1324),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(2266, 3128),
   Score_Pair(1744, 2917),
   Score_Pair(1597, 2408),
   Score_Pair(1753, 2220),
   Score_Pair(361, 1621),
   Score_Pair(1091, 1735),
   Score_Pair(1680, 1684),
   Score_Pair(797, 558),
   Score_Pair(1474, 724),
   Score_Pair(1737, 951),
   Score_Pair(1526, 1488),
   Score_Pair(-1309, 1911),
   Score_Pair(2069, 3730),
   Score_Pair(1174, 2816),
   Score_Pair(340, 2310),
   Score_Pair(256, 2100),
   Score_Pair(1526, 2026),
   Score_Pair(1860, 1347),
   Score_Pair(748, 672),
   Score_Pair(177, 632),
   Score_Pair(310, 974),
   Score_Pair(682, 1392),
   Score_Pair(333, 1855),
   Score_Pair(-1797, 2057),
   Score_Pair(1590, 3157),
   Score_Pair(920, 2918),
   Score_Pair(276, 2766),
   Score_Pair(590, 2706),
   Score_Pair(1156, 985),
   Score_Pair(852, 1443),
   Score_Pair(551, 2004),
   Score_Pair(-308, 1822),
   Score_Pair(496, 1584),
   Score_Pair(261, 1148),
   Score_Pair(-194, 899),
   Score_Pair(561, 1662),
   Score_Pair(2170, 1368),
   Score_Pair(1551, 1402),
   Score_Pair(982, 1343),
   Score_Pair(816, 1446),
   Score_Pair(1530, -1092),
   Score_Pair(1189, -438),
   Score_Pair(448, 709),
   Score_Pair(274, 1354),
   Score_Pair(386, 1610),
   Score_Pair(587, 1426),
   Score_Pair(130, 1484),
   Score_Pair(974, 505),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(5135, 5285),
   Score_Pair(5381, 6995),
   Score_Pair(5998, 6099),
   Score_Pair(3863, 4189),
   Score_Pair(6184, 6120),
   Score_Pair(-598, 2435),
   Score_Pair(0, 0),
   Score_Pair(3226, 3735),
   Score_Pair(5583, 4777),
   Score_Pair(3666, 5480),
   Score_Pair(8205, 4303),
   Score_Pair(171, 2484),
   Score_Pair(2380, 3553),
   Score_Pair(0, 0),
   Score_Pair(4987, 5564),
   Score_Pair(4548, 5494),
   Score_Pair(5338, 6323),
   Score_Pair(33, 3105),
   Score_Pair(1059, 2978),
   Score_Pair(1487, 3035),
   Score_Pair(0, 0),
   Score_Pair(5507, 5818),
   Score_Pair(7837, 4619),
   Score_Pair(-11, 2642),
   Score_Pair(166, 1979),
   Score_Pair(183, 3359),
   Score_Pair(-18, 2364),
   Score_Pair(0, 0),
   Score_Pair(8547, 9148),
   Score_Pair(1350, 5324),
   Score_Pair(1993, 3158),
   Score_Pair(3238, 3139),
   Score_Pair(1606, 1915),
   Score_Pair(-2865, -2087),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(4846, 84),
   Score_Pair(1896, 2292),
   Score_Pair(4968, 563),
   Score_Pair(4483, -381),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(-4467, -5814),
   Score_Pair(-3922, -2676),
   Score_Pair(-3369, -2734),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(-1407, -1194),
   Score_Pair(-4183, -3416),
   Score_Pair(-2544, -2264),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(-9909, -8818),
   Score_Pair(-2359, -1914),
   Score_Pair(-3156, -3071),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(-1421, -1029),
   Score_Pair(-5470, -4098),
   Score_Pair(-1944, -2004),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(1377, -856),
   Score_Pair(820, 106),
   Score_Pair(1200, -193),
   Score_Pair(411, 41),
   Score_Pair(345, -392),
   Score_Pair(119, -59),
   Score_Pair(1507, 328),
   Score_Pair(-235, 988),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(3108, -1492),
   Score_Pair(1610, -411),
   Score_Pair(2140, -938),
   Score_Pair(844, -319),
   Score_Pair(1670, -472),
   Score_Pair(841, -199),
   Score_Pair(2044, 2019),
   Score_Pair(242, 3736),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(2827, -733),
   Score_Pair(2429, 185),
   Score_Pair(2428, -276),
   Score_Pair(1368, 297),
   Score_Pair(2263, 365),
   Score_Pair(1464, -87),
   Score_Pair(3734, 3513),
   Score_Pair(1875, 4100),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(1650, 1078),
   Score_Pair(2571, 1805),
   Score_Pair(2612, 1809),
   Score_Pair(2117, 1475),
   Score_Pair(2582, 1825),
   Score_Pair(2038, 1330),
   Score_Pair(3092, 2311),
   Score_Pair(3057, 2666),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(135, -591),
   Score_Pair(2888, -242),
   Score_Pair(1319, 1043),
   Score_Pair(607, 1425),
   Score_Pair(0, 0),
   Score_Pair(744, -1340),
   Score_Pair(4, -1722),
   Score_Pair(31, -688),
   Score_Pair(-662, -577),
   Score_Pair(947, 1200),
   Score_Pair(3785, 4814),
   Score_Pair(0, 0),
   Score_Pair(782, 187),
   Score_Pair(1421, 1757),
   Score_Pair(1681, 1413),
   Score_Pair(1396, 1889),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(-676, -216),
   Score_Pair(-1208, -931),
   Score_Pair(-401, -284),
   Score_Pair(1420, 629),
   Score_Pair(2001, 1675),
   Score_Pair(4079, 4480),
   Score_Pair(223, 139),
   Score_Pair(1145, 386),
   Score_Pair(1263, 746),
   Score_Pair(1271, 854),
   Score_Pair(-613, -11),
   Score_Pair(-752, -305),
   Score_Pair(-558, 111),
   Score_Pair(215, -37),
   Score_Pair(307, 670),
   Score_Pair(982, 754),
   Score_Pair(1674, 1084),
   Score_Pair(2649, -140),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(2020, 670),
   Score_Pair(1918, 1426),
   Score_Pair(1277, 397),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(3061, 2658),
   Score_Pair(2017, 1496),
   Score_Pair(900, 338),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(-516, -940),
   Score_Pair(-265, -2),
   Score_Pair(1976, -1328),
   Score_Pair(692, -870),
   Score_Pair(-3020, -3391),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(-109, 1039),
   Score_Pair(-185, 352),
   Score_Pair(-2298, 901),
   Score_Pair(-1038, 598),
   Score_Pair(928, 1673),
   Score_Pair(312, 1625),
   Score_Pair(797, 562),
   Score_Pair(47, -578),
   Score_Pair(-1127, -543),
   Score_Pair(-946, -459),
   Score_Pair(621, -431),
   Score_Pair(-742, -815),
   Score_Pair(1205, 1749),
   Score_Pair(680, 780),
   Score_Pair(-212, 667),
   Score_Pair(102, -289),
   Score_Pair(684, -685),
   Score_Pair(-472, -338),
   Score_Pair(-981, -27),
   Score_Pair(-1337, -1007),
   Score_Pair(565, 3196),
   Score_Pair(80, 1534),
   Score_Pair(-2185, 2377),
   Score_Pair(-2186, 373),
   Score_Pair(2971, 3870),
   Score_Pair(1995, 3921),
   Score_Pair(451, 1829),
   Score_Pair(-725, -112),
   Score_Pair(-1031, -1996),
   Score_Pair(-1304, -1788),
   Score_Pair(-316, -1151),
   Score_Pair(-1491, -1325),
   Score_Pair(720, 912),
   Score_Pair(-666, -1704),
   Score_Pair(842, -1414),
   Score_Pair(-451, -1047),
   Score_Pair(-616, 1203),
   Score_Pair(166, 1877),
   Score_Pair(279, 1820),
   Score_Pair(286, 1560),
   Score_Pair(1701, 5046),
   Score_Pair(-48, 2378),
   Score_Pair(534, 4380),
   Score_Pair(-1517, 1200),
   Score_Pair(2345, 3083),
   Score_Pair(4659, 6182),
   Score_Pair(1535, 2268),
   Score_Pair(-133, 149),
   Score_Pair(-2431, -1162),
   Score_Pair(-1876, -2375),
   Score_Pair(-1134, -1941),
   Score_Pair(-1425, -1209),
   Score_Pair(-624, -975),
   Score_Pair(490, -3970),
   Score_Pair(526, -2548),
   Score_Pair(-953, 132),
   Score_Pair(-500, 2441),
   Score_Pair(429, 3835),
   Score_Pair(942, 3423),
   Score_Pair(1363, 2668),
   Score_Pair(3510, 6265),
   Score_Pair(146, 4230),
   Score_Pair(4501, 7460),
   Score_Pair(-64, 896),
   Score_Pair(908, 1259),
   Score_Pair(5160, 7298),
   Score_Pair(2416, 2970),
   Score_Pair(1369, 872),
   Score_Pair(-634, -1076),
   Score_Pair(-1677, -1880),
   Score_Pair(-2703, -1732),
   Score_Pair(-1424, -1363),
   Score_Pair(-1977, -3909),
   Score_Pair(200, -6206),
   Score_Pair(260, -3536),
   Score_Pair(-429, 1632),
   Score_Pair(118, 4613),
   Score_Pair(939, 5260),
   Score_Pair(1457, 4303),
   Score_Pair(3029, 4332),
   Score_Pair(7339, 10839),
   Score_Pair(5339, 6405),
   Score_Pair(8402, 10906),
   Score_Pair(523, 745),
   Score_Pair(-1962, -2745),
   Score_Pair(3586, 4978),
   Score_Pair(2859, 3521),
   Score_Pair(2981, 3521),
   Score_Pair(517, 398),
   Score_Pair(761, -20),
   Score_Pair(-44, 239),
   Score_Pair(-1326, 1250),
   Score_Pair(-6093, -5592),
   Score_Pair(-3591, -4967),
   Score_Pair(-928, -3216),
   Score_Pair(1887, 2700),
   Score_Pair(3601, 5760),
   Score_Pair(4401, 6313),
   Score_Pair(4153, 5870),
   Score_Pair(3571, 3806),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(679, -264),
   Score_Pair(115, 247),
   Score_Pair(-95, 464),
   Score_Pair(-274, 807),
   Score_Pair(881, 79),
   Score_Pair(356, 402),
   Score_Pair(786, 382),
   Score_Pair(1074, 748),
   Score_Pair(-614, 164),
   Score_Pair(1068, 1635),
   Score_Pair(435, 1155),
   Score_Pair(1201, 2215),
   Score_Pair(555, 1601),
   Score_Pair(2713, 3277),
   Score_Pair(2660, 2873),
   Score_Pair(3024, 4183),
   Score_Pair(2906, 3656),
   Score_Pair(5208, 6660),
   Score_Pair(4170, 5632),
   Score_Pair(4697, 6186),
   Score_Pair(632, 853),
   Score_Pair(1066, 1457),
   Score_Pair(588, 782),
   Score_Pair(590, 819),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(1689, 1277),
   Score_Pair(1543, 890),
   Score_Pair(1237, 1303),
   Score_Pair(1475, 1857),
   Score_Pair(20, 1238),
   Score_Pair(977, 1128),
   Score_Pair(1646, 937),
   Score_Pair(1799, 1644),
   Score_Pair(235, 1369),
   Score_Pair(1428, 1643),
   Score_Pair(2008, 2094),
   Score_Pair(2791, 2343),
   Score_Pair(2194, 4559),
   Score_Pair(3442, 4838),
   Score_Pair(5197, 4822),
   Score_Pair(5347, 5690),
   Score_Pair(1680, 2397),
   Score_Pair(3078, 4090),
   Score_Pair(2879, 3642),
   Score_Pair(1294, 1740),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(1891, 858),
   Score_Pair(782, 685),
   Score_Pair(836, 910),
   Score_Pair(1536, 1324),
   Score_Pair(1285, 279),
   Score_Pair(1073, 309),
   Score_Pair(1070, 393),
   Score_Pair(971, 666),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(-1285, -279),
   Score_Pair(-1073, -309),
   Score_Pair(-1070, -393),
   Score_Pair(-971, -666),
   Score_Pair(-1891, -858),
   Score_Pair(-782, -685),
   Score_Pair(-836, -910),
   Score_Pair(-1536, -1324),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(0, 0),
   Score_Pair(-278, 208),
   Score_Pair(-665, -812),
   Score_Pair(-642, -316),
   Score_Pair(-781, -241),
   Score_Pair(0, 0),
   Score_Pair(-307, -105),
   Score_Pair(-132, -476),
   Score_Pair(-650, 66),
   Score_Pair(-175, -276),
   Score_Pair(-484, -209),
   Score_Pair(-619, -163),
   Score_Pair(0, 0),
   Score_Pair(-2523, -1052),
   Score_Pair(1641, -2392),
   Score_Pair(-67, -403),
   Score_Pair(-505, -1184),
   Score_Pair(1361, 1467),
};

// variables

std::vector<Pawn_Info> G_Pawn_Table;

// prototypes

static int  eval (const Pos & pos);

static void comp_pawn_info (Pawn_Info & pi, const Pos & pos);

static bool two_knights    (const Pos & pos, Side sd);
static bool rook_pawn_draw (const Pos & pos, Side sd, File fl);

static Bit  king_zone (Square sq, Side sd);

static bool pawn_is_free        (const Pos & pos, Square sq, Side sd, const Attack_Info & ai);
static bool pawn_is_unstoppable (const Pos & pos, Square sq, Side sd, const Attack_Info & ai);

// functions

void clear_pawn_table() {

   Pawn_Info entry {
      Key(1),
      { Score_Pair(0), Score_Pair(0) },
      { Bit(0), Bit(0) },
      { Bit(0), Bit(0) },
      0.0, 0.0,
   };

   G_Pawn_Table.clear();
   G_Pawn_Table.resize(Pawn_Table_Size, entry);
}

Score eval(const Pos & pos, Side sd) {

   int sc = eval(pos);

   // drawish?

   Side win  = (sc >= 0) ? White : Black;
   Side lose = side_opp(win);

   int fw = pos::force(pos, win);
   int fl = pos::force(pos, lose);

   if (fw < 6) {

      Bit pw = pos.pawns(win);
      Bit pl = pos.pawns(lose);

      Bit minors = pos.pieces(Knight, lose) | pos.pieces(Bishop, lose);

      if (false) {

      } else if (fw == 0 && pw == 0) { // lone king

         sc = 0;

      } else if (rook_pawn_draw(pos, win, File_A)) {

         sc /= 64;

      } else if (rook_pawn_draw(pos, win, File_H)) {

         sc /= 64;

      } else if (pw == 0) {

         if (fw <= 1) { // insufficient material
            sc /= 16;
         } else if (fw == 2 && two_knights(pos, win) && pl == 0) {
            sc /= 16;
         } else if (fw - fl <= 1) {
            sc /= 4;
         }

      } else if (bit::is_single(pw)) {

         Square pawn = bit::first(pw);
         bool blocked = (pawn::file(pawn) & pawn::fronts(pawn, win) & pos.pieces(King, lose)) != 0;

         if (fw <= 1 && minors != 0) { // minor sacrifice
            sc /= 8;
         } else if (fw == 2 && two_knights(pos, win) && pl == 0 && minors != 0) { // minor sacrifice
            sc /= 8;
         } else if (fw == fl && blocked) { // blocked by king
            sc /= 4;
         } else if (fw == fl && minors != 0) { // minor sacrifice
            sc /= 2;
         }

      } else if (pos::opposit_bishops(pos) && std::abs(pos.count(Pawn, White) - pos.count(Pawn, Black)) <= 2) {

         sc /= 2;
      }
   }

   return score::clamp(score::side(Score(sc), sd)); // for sd
}

static int eval(const Pos & pos) {

   Key key = pos.key_pawn();
   Pawn_Info & entry = G_Pawn_Table[hash::index(key, Pawn_Table_Mask)];

   if (entry.key != key) {
      comp_pawn_info(entry, pos);
      entry.key = key;
   }

   Pawn_Info pi = entry;

   Attack_Info ai;
   ai.init(pos);

   Score_Pair sc;

   for (int s = 0; s < Side_Size; s++) {

      Side sd = side_make(s);
      Side xd = side_opp(sd);

      Bit pawns_sd = pos.pawns(sd);
      Bit pawns_xd = pos.pawns(xd);

      Square king_sd = pos.king(sd);
      Square king_xd = pos.king(xd);

      Bit king_zone_sd = king_zone(king_sd, sd);
      Bit king_zone_xd = king_zone(king_xd, xd);

      int var;

      // material

      var = 0;

      for (int p = Pawn; p <= Queen; p++) {

         Piece pc = piece_make(p);

         int mat = pos.count(pc, sd);
         sc += W[var + pc] * mat;
      }

      var = 6;

      if (pos.count(Bishop, sd) > 1) sc += W[var];

      // pawns

      Piece pc = Pawn;

      Bit blocked_sd = pawn::blocked(pos, sd);

      sc += pi.score[sd]; // pawn-only score

      // pawn mobility

      var = 199 + pc * 12;

      int mob = bit::count(bit::pawn_moves(sd, pawns_sd) & pos.empties());
      sc += W[var] * mob;

      // pawn captures

      var = 271 + pc * Piece_Size;

      for (Bit b = ai.pawn_attacks(sd) & pos.non_pawns(xd); b != 0; b = bit::rest(b)) {
         Square to = bit::first(b);
         sc += W[var + pos.piece(to)];
      }

      // passed pawns

      for (Bit b = pi.passed[sd] & pos.pawns(sd); b != 0; b = bit::rest(b)) {

         Square sq = bit::first(b);

         Rank rank = std::max(square_rank(sq, sd), Rank_3);

         if (pawn_is_unstoppable(pos, sq, sd, ai)) {

            int gain = piece_mat(Queen) - piece_mat(Pawn);
            sc += Score_Pair(gain * Scale * (rank - Rank_2) / 6);

         } else {

            var = 486 + rank * 20;

            Square stop = square_front(sq, sd);

            sc += W[var + 0];
            if (!pos.is_side(stop, xd))        sc += W[var + 1];
            if (pawn_is_free(pos, sq, sd, ai)) sc += W[var + 2];
            sc += W[var +  4 + square_dist(king_sd, stop)];
            sc += W[var + 12 + square_dist(king_xd, stop)];
         }
      }

      // pawn shield

      var = 397;

      for (Bit b = pawns_sd & king_zone_sd & ~pawn::rears(king_sd, sd); b != 0; b = bit::rest(b)) {

         Square sq = bit::first(b);

         File fl = square_file(sq);
         Rank rk = square_rank(sq, sd);

         if (fl >= File_Size / 2) fl = file_opp(fl);

         sc += W[var + 0 + fl];
         sc += W[var + 4 + rk];
      }

      // pieces

      Bit pawn_safe = ~(pawns_sd | ai.pawn_attacks(xd));

      int attackers = 0;

      for (Bit b = pos.non_king(sd); b != 0; b = bit::rest(b)) {
         Square sq = bit::first(b);
         if ((ai.piece_attacks(sq) & king_zone_xd & pawn_safe) != 0) attackers += 1;
      }

      attackers = std::min(attackers, 4);

      // piece loop

      for (Bit b = pos.non_king(sd); b != 0; b = bit::rest(b)) {

         Square sq = bit::first(b);
         Piece  pc = pos.piece(sq);

         assert(pc != King);

         File fl = square_file(sq);
         Rank rk = square_rank(sq, sd);

         if (fl >= File_Size / 2) fl = file_opp(fl);

         Bit tos = ai.piece_attacks(sq);

         // position

         var = 7 + pc * 32;

         sc += W[var + rk * 4 + fl];

         // mobility

         var = 199 + pc * 12;

         int mob = bit::count(tos & pawn_safe);
         sc += (W[var + 0 + fl] + W[var + 4 + rk]) * math::sqrt(mob);

         // captures

         var = 271 + pc * Piece_Size;

         for (Bit bt = tos & pos.pieces(xd) & ~(pawns_xd & ai.pawn_attacks(xd)); bt != 0; bt = bit::rest(bt)) {
            Square to = bit::first(bt);
            sc += W[var + pos.piece(to)];
         }

         // checks

         if (!bit::has(tos, king_xd)) { // skip if already giving check

            var = 307;

            int check = 0;

            for (Bit bt = tos & ~pos.pieces(sd) & bit::piece_attacks_to(pc, sd, king_xd) & ai.queen_safe(sd); bt != 0; bt = bit::rest(bt)) {

               Square to = bit::first(bt);

               assert(bit::line_is_empty(sq, to, pos.pieces()));
               if (bit::line_is_empty(to, king_xd, pos.pieces())) check += 1;
            }

            sc += W[var + pc] * check;
         }

         // pinned?

         if (is_pinned(pos, king_sd, sq, sd)) {

            var = 313 + pc * Piece_Size;

            Square pin_sq = pinned_by(pos, king_sd, sq, sd);
            Piece  pin_pc = pos.piece(pin_sq);

            if (pin_pc <= pc || (tos & bit::ray(king_sd, sq)) == 0) { // can't move
               sc += W[var + pin_pc];
            }
         }

         // king attack

         if (attackers != 0) {

            assert(attackers >= 1 && attackers <= 4);

            var = 349 + ((attackers - 1) * Piece_Size + pc) * 2;

            int p0 = bit::count(tos & king_zone_xd & pawn_safe & ~ai.queen_attacks(xd));
            int p1 = bit::count(tos & king_zone_xd & pawn_safe &  ai.queen_attacks(xd));

            sc += W[var + 0] * p0;
            sc += W[var + 1] * p1;
         }

         // defended piece

         if (pc != Queen && bit::has(ai.attacks(sd), sq)) {

            var = 409;

            if (pawn::is_protected(pos, sq, sd)) { // by pawn
               sc += W[var +  0 + fl];
               sc += W[var +  4 + rk];
            } else { // by piece
               sc += W[var + 12 + fl];
               sc += W[var + 16 + rk];
            }
         }

         // minor outpost

         if (piece_is_minor(pc)
          && pawn::is_protected(pos, sq, sd)
          && bit::has(pi.strong[sd], sq)
          && (fl >= File_C && fl <= File_F)
          && (rk >= Rank_4 && rk <= Rank_6)
          ) {

            var = 433;

            sc += W[var + pc * Rank_Size + rk];
         }

         // knight distance to pawns

         if (pc == Knight) {

            var = 756;

            double knight_file = double(square_file(sq)) + 0.5;
            double knight_rank = double(square_rank(sq)) + 0.5;

            double df = std::abs(knight_file - double(pi.centre_file));
            double dr = std::abs(knight_rank - double(pi.centre_rank));

            sc += W[var + 0] * df;
            sc += W[var + 1] * dr;
         }

         // bad bishop

         if (pc == Bishop) {

            var = 481;

            Bit bad_pawns = pawns_sd & bit::Colour_Squares[square_colour(sq)];

            int p0 = bit::count(bad_pawns &  blocked_sd);
            int p1 = bit::count(bad_pawns & ~blocked_sd);

            sc += W[var + 0] * p0;
            sc += W[var + 1] * p1;
         }

         // rook on open file

         if (pc == Rook && pawn::is_open(pos, sq, sd)) {

            var = 483;

            if (pawn::is_open(pos, sq, xd)) { // open
               sc += W[var + 0];
            } else { // semi-open
               sc += W[var + 1];
            }
         }

         // rook blocked by own king

         if (pc == Rook
          && bit::count(tos & pawn_safe & ~pos.pieces(King, sd)) < 3
          && rk < Rank_3
          && square_rank(king_sd, sd) == Rank_1
          && !bit::has(pos.castling_rooks(sd), sq)
          && (square_file(king_sd) < File_Size / 2
            ? square_file(sq) <= square_file(king_sd)
            : square_file(sq) >= square_file(king_sd))
          ) {

            var = 485;

            sc += W[var];
         }
      }

      // king

      {
         Square sq = pos.king(sd);
         Piece  pc = King;

         File fl = square_file(sq);
         Rank rk = square_rank(sq, sd);

         if (fl >= File_Size / 2) fl = file_opp(fl);

         Bit tos = ai.piece_attacks(sq);

         // position

         var = 7 + pc * 32;

         sc += W[var + rk * 4 + fl];

         // captures

         var = 271 + pc * Piece_Size;

         for (Bit bt = tos & pos.pieces(xd) & ~ai.pawn_attacks(xd); bt != 0; bt = bit::rest(bt)) {
            Square to = bit::first(bt);
            sc += W[var + pos.piece(to)];
         }

         // distance to pawns

         var = 754;

         double king_file = double(square_file(sq)) + 0.5;
         double king_rank = double(square_rank(sq)) + 0.5;

         double df = std::abs(king_file - double(pi.centre_file));
         double dr = std::abs(king_rank - double(pi.centre_rank));

         sc += W[var + 0] * df;
         sc += W[var + 1] * dr;
      }

      // side-to-move bonus

      if (sd == pos.turn() && !bit::has(ai.attacks(xd), king_sd)) {

         var = 758;

         sc += W[var];
      }

      // prepare for opponent

      sc = -sc;
   }

   // game phase

   int stage = pos::stage(pos);

   return ml::div_round(sc.mg() * (Stage_Size - stage) + sc.eg() * stage, Stage_Size * Scale); // unit -> cp
}

static void comp_pawn_info(Pawn_Info & pi, const Pos & pos) {

   for (int sd = 0; sd < Side_Size; sd++) {
      pi.passed[sd] = Bit(0);
      pi.strong[sd] = pawn::strong(pos, Side(sd));
   }

   pi.centre_file = 0.0;
   pi.centre_rank = 0.0;

   for (int s = 0; s < Side_Size; s++) {

      Side sd = side_make(s);

      Score_Pair sc;

      int var;

      // init

      Piece pc = Pawn;

      Bit weak_sd = pawn::weak(pos, sd);

      // pawn loop

      for (Bit b = pos.pawns(sd); b != 0; b = bit::rest(b)) {

         Square sq = bit::first(b);

         File fl = square_file(sq);
         Rank rk = square_rank(sq, sd);

         if (fl >= File_Size / 2) fl = file_opp(fl);

         // position

         var = 7 + pc * 32;

         sc += W[var + rk * 4 + fl];

         // space

         var = 646;

         if (pawn::is_duo      (pos, sq, sd)) sc += W[var +  0 + rk * 4 + fl];
         if (pawn::is_protected(pos, sq, sd)) sc += W[var + 32 + rk * 4 + fl];
         if (pawn::is_ram      (pos, sq, sd)) sc += W[var + 64 + rk * 4 + fl];

         // weak?

         if (bit::has(weak_sd, sq)) {

            var = 742;

            sc += W[var + 0 + fl];
            sc += W[var + 4 + rk];
         }

         // passed?

         if (pawn::is_passed(pos, sq, sd)) bit::set(pi.passed[sd], sq);

         // centre

         pi.centre_file += double(square_file(sq)) + 0.5;
         pi.centre_rank += double(square_rank(sq)) + 0.5;
      }

      pi.score[sd] = sc;
   }

   int pawn_size = bit::count(pos.pieces(Pawn));

   if (pawn_size == 0) { // no pawns => board centre
      pi.centre_file = double(File_Size) / 2.0;
      pi.centre_rank = double(Rank_Size) / 2.0;
   } else {
      pi.centre_file /= double(pawn_size);
      pi.centre_rank /= double(pawn_size);
   }
}

static bool two_knights(const Pos & pos, Side sd) {

   Bit pieces = pos.non_king(sd);
   if (pieces != pos.pieces(Knight, sd)) return false;

   if (bit::count(pieces) != 2) return false;

   return true;
}

static bool rook_pawn_draw(const Pos & pos, Side sd, File fl) {

   Bit pawns = pos.pawns(sd);
   if (pawns == 0 || !bit::is_incl(pawns, bit::file(fl))) return false;

   Bit bishops = pos.non_king(sd);
   if (bishops != pos.pieces(Bishop, sd)) return false;

   Square prom = square_make(fl, Rank_8, sd);

   Side xd = side_opp(sd);
   if (square_dist(pos.king(xd), prom) > 1) return false;

   Bit squares = bit::Colour_Squares[square_colour(prom)];
   if ((bishops & squares) != 0) return false;

   return true;
}

static Bit king_zone(Square sq, Side sd) {

   File fl = math::clamp(square_file(sq), File_B, File_G);
   Rank rk = math::clamp(square_rank(sq), Rank_2, Rank_7);

   sq = square_make(fl, rk);
   return bit::bit(sq) | bit::piece_attacks(King, sd, sq); // 3x3 square
}

static bool pawn_is_free(const Pos & pos, Square sq, Side sd, const Attack_Info & ai) {

   Side xd = side_opp(sd);

   Square stop = square_front(sq, sd);
   Bit fronts = pawn::file(sq) & pawn::fronts(sq, sd);

   return (fronts & (pos.pieces(xd) | ~ai.queen_safe(sd))) == 0 // free path
       && !is_pinned(pos, stop, sq, sd); // not attacked from behind by major
}

static bool pawn_is_unstoppable(const Pos & pos, Square sq, Side sd, const Attack_Info & ai) {

   Side xd = side_opp(sd);
   if (!pos::lone_king(pos, xd)) return false;

   Bit fronts = pawn::file(sq) & pawn::fronts(sq, sd);
   if ((fronts & pos.pieces()) != 0) return false;

   Square king_sd = pos.king(sd);
   Square king_xd = pos.king(xd);

   Rank rk   = square_rank(sq, sd);
   Rank rank = std::max(rk, Rank_3);

   Square prom = square_prom(sq, sd);

   int md = Rank_8 - rank;
   int od = square_dist(king_xd, prom);

   if (pos.turn() == xd) md += 1;

   return md < od // faster than opponent king
       || bit::is_incl(fronts, ai.piece_attacks(king_sd)); // protected path
}

Score piece_mat(Piece pc) {

   assert(pc != Piece_None);

   const int mat[Piece_Size + 1] { 100, 325, 325, 500, 1000, 10000, 0 };
   return Score(mat[pc]);
}

Score_Pair::Score_Pair() : Score_Pair(0) {
}

Score_Pair::Score_Pair(int sc) : Score_Pair(sc, sc) {
}

Score_Pair::Score_Pair(int mg, int eg) {
   p_vec = (int64(mg) << 32) + int64(eg); // HACK: "eg"'s sign leaks to "mg"
}

void Score_Pair::operator+=(Score_Pair sp) {
   p_vec += sp.p_vec;
}

void Score_Pair::operator-=(Score_Pair sp) {
   p_vec -= sp.p_vec;
}

int Score_Pair::mg() const {
   return p_vec >> 32;
}

int Score_Pair::eg() const {
   return int(p_vec); // extend sign
}

Score_Pair Score_Pair::make(int64 vec) {
   Score_Pair sp;
   sp.p_vec = vec;
   return sp;
}

Score_Pair operator+(Score_Pair sp) {
   return Score_Pair::make(+sp.p_vec);
}

Score_Pair operator-(Score_Pair sp) {
   return Score_Pair::make(-sp.p_vec);
}

Score_Pair operator+(Score_Pair s0, Score_Pair s1) {
   return Score_Pair::make(s0.p_vec + s1.p_vec);
}

Score_Pair operator-(Score_Pair s0, Score_Pair s1) {
   return Score_Pair::make(s0.p_vec - s1.p_vec);
}

Score_Pair operator*(Score_Pair weight, int n) {
   return Score_Pair::make(weight.p_vec * n);
}

Score_Pair operator*(Score_Pair weight, double x) {
   return Score_Pair(ml::round(double(weight.mg()) * x),
                     ml::round(double(weight.eg()) * x));
}

