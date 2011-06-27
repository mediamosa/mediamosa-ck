<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * Static class for helper functions.
 */

class MediaMediaMosa {
  /**
   * Check and validate a datestamp string for metadata.
   * returns FALSE or valid string to use
   *
   * @param string $str
   * @return string or FALSE
   */
  static public function check_datestamp($str_date) {

    $str_date = trim($str_date);
    if ($str_date == '') {
      return '0000-00-00 00:00:00'; // Empty is ok
    }

    if (!preg_match('/^\d{4}-\d{2}-\d{2}( \d{2}:\d{2}:\d{2})?$/', $str_date)) {
      return FALSE;
    }

    if (strlen($str_date) == 10) {
      $str_date .= ' 00:00:00';
    }

    list($year, $month, $day, $hour, $minute, $second) = preg_split('/[:\- ]{1}/', $str_date);

    if (!checkdate($month, $day, $year)) {
      return FALSE;
    }

    if ($hour > 23 || $minute > 59 || $second > 59) {
      return FALSE;
    }

    return $str_date;
  }
}