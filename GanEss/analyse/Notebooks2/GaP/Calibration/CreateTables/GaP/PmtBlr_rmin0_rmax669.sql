DROP TABLE IF EXISTS `PmtBlr`;
CREATE TABLE `PmtBlr` (
  `MinRun` int(11) NOT NULL,
  `MaxRun` int(11) DEFAULT NULL,
  `ElecID` int(11) NOT NULL,
  `coeff_c` double NOT NULL,
  `coeff_blr` double NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

