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
 * All related user / admin functions (no hooks).
 */

/**
 * Is called by redirect when upload was done by MediaMosa.
 *
 * @param $asset_id
 *   The ID of the asset.
 * @param $mediafile_id
 *   The ID of the mediafile.
 */
function _media_mediamosa_uploaded_file($asset_id, $mediafile_id) {

  $fid = 0;

  try {
    // Need to get info of mediafile/asset.
    $mediafile_xml = MediaMosaCkConnectorWrapper::get_mediafile($mediafile_id);

    // Get filename.
    $filename = (string) $mediafile_xml->items->item->filename;

    // We need to build the scheme uri. (mediamosa://asset/[id]/[id]/[filename]).
    $uri = strtr('mediamosa://asset/@asset_id/@mediafile_id/@filename', array('@asset_id' => $asset_id, '@mediafile_id' => $mediafile_id, '@filename' => $filename));

    // Save it using uri.
    $file = file_uri_to_object($uri);
    file_save($file);

    // Get the file ID.
    $fid = $file->fid;
  }
  catch (Exception $e) {
    // If we fail, then don't create a file.
  }

  // Need to do this to close the popup.
  call_user_func_array('drupal_goto', array('media/browser', array('query' => array('render' => 'media-popup', 'fid' => $fid))));
}
