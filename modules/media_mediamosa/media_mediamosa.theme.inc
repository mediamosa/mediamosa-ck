<?php
/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * All theming functions.
 */

function theme_media_mediamosa_link($variables) {
  // Based on the asset_id and mediafile_id we will try to find the file in the
  // file_managed table.
  $uri = strtr('mediamosa://asset/@asset_id/%/%', array('@asset_id' => $variables['asset_id']));

  // Find in the file managed.
  $file_managed = db_select('file_managed', 'fm')
    ->fields('fm', array('fid', 'uri'))
    ->condition('uri', $uri, 'LIKE')
    ->range(0, 1)
    ->execute()->fetchAssoc();

  if (!$file_managed) {
    return t('Broken file');
  }

  // Get parts.
  $parts = MediaMosaStreamWrapper::mediamosa_parse_url($file_managed['uri']);

  // Return link to media.
  return l(check_plain($parts['filename']), 'file/' . $file_managed['fid']);
}

/**
 * Preview for Styles UI.
 */
function theme_media_mediamosa_preview_style($variables) {
  // TODO
  watchdog('media_mediamosa', t('Fixme: Style'), array(), WATCHDOG_WARNING);
  $variables['uri'] = 'FIXME: Theme';
  $variables['field_type'] = 'file';
  $variables['object'] = file_uri_to_object($variables['uri']);
  return theme('styles', $variables);
}
