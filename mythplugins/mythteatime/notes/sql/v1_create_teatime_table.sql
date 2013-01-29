delimiter $$

CREATE TABLE `teatime` (
  `id` MEDIUMINT NOT NULL AUTO_INCREMENT,
   PRIMARY KEY(`id`),
  `message_text` text,
  `timer_type` enum('date_time','time_span') DEFAULT NULL,
  `run_date_time` timestamp NULL DEFAULT NULL,
  `time_span` time DEFAULT NULL,
  `system_key_events` text,
  `jump_points` text,
  `pause_playback` enum('no','yes') DEFAULT NULL,
  `countdown_seconds` int(11) DEFAULT NULL
  COMMENT 'stores timer data for the teatime plugin'
) ENGINE=MyISAM DEFAULT CHARSET=utf8$$

