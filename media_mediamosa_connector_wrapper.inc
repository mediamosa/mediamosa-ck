<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Embedded Video Field provider file for MediaMosa.org connector helpers.
 */

/**
 * MediaMosa connector request wrapper.
 *
 * @param $uri
 *   REST uri
 *
 * @param $options
 *   Options
 *
 * @see mediamosa connector class request() function.
 *
 * @return
 *   The result object, or FALSE in case of error.
 */
function _media_mediamosa_connector($uri, array $options = array()) {
  // Check the parameters.
  if (empty($uri)) {
    return FALSE;
  }

  // Turn fatal TRUE to get all the errors.
  $options['fatal'] = FALSE;

  try {
     // Mediamosa connector.
    $mediamosa_connectors = new mediamosa_connector();

    // Make the request.
    if (!empty($options['method']) && $options['method'] == 'POST') {
      $response = $mediamosa_connectors->request_post($uri, $options);
    }
    else {
      $response = $mediamosa_connectors->request_get($uri, $options);
    }
  }
  catch (Exception $e) {
    drupal_set_message('There was an exception while connecting to MediaMosa. Please, check you connection.');
    watchdog('media_mediamosa', 'There was an exception while connecting to MediaMosa. Please, check you connection. URI = @uri, $options = @options', array(
      '@uri' => print_r($uri, TRUE),
      '@options' => print_r($options, TRUE),
    ), WATCHDOG_ERROR);
    //drupal_goto('admin/mediamosa/config/connector');
    return FALSE;
  }

  if (!$response) {
    drupal_set_message('There was a problem while connecting to MediaMosa. Please, check you connection.');
    watchdog('media_mediamosa', 'There was a problem while connecting to MediaMosa. Please, check you connection. URI = @uri, $options = @options', array(
      '@uri' => print_r($uri, TRUE),
      '@options' => print_r($options, TRUE),
    ), WATCHDOG_ERROR);
    //drupal_goto('admin/mediamosa/config/connector');
    return FALSE;
  }

  return $response;
}

/**
 * Asset create.
 *
 * @param $user_id
 *
 * @return $asset_id
 */
function _media_mediamosa_asset_create($user_id) {
  // Check the parameters.
  if (empty($user_id)) {
    return FALSE;
  }

  $data = array(
    'user_id' => $user_id,
  );

  $response = _media_mediamosa_connector('asset/create', array('method' => 'POST', 'data' => $data,));

  if (!$response) {
    drupal_set_message(t('Error creating asset.'), 'error');
    watchdog('media_mediamosa', t('Error creating asset.'), array(), WATCHDOG_ERROR);
    return FALSE;
  }

  $asset_id = (string) $response->xml->items->item->asset_id;

  return $asset_id;
}

/**
 * Metadata create.
 *
 * @param $asset_id
 * @param $user_id
 * @param $title
 * @param $description
 *
 * @return boolean
 *   TRUE in case of success, or FALSE in the other case.
 */
function _media_mediamosa_metadata_create($asset_id, $user_id, $title, $description, $action = 'update') {
  // Check the parameters.
  if (empty($asset_id) || empty($user_id)) {
    return FALSE;
  }

  if (!$title && !$description) {
    return FALSE;
  }

  $data = array(
    'user_id' => $user_id,
    'action' => $action,
  );
  if ($title) {
    $data['title'] = $title;
  }
  if ($description) {
    $data['description'] = $description;
  }

  $response = _media_mediamosa_connector("asset/$asset_id/metadata", array('method' => 'POST', 'data' => $data,));

  if (!$response) {
    drupal_set_message(t('Error creating asset metadata.'), 'error');
    watchdog('media_mediamosa', t('Error creating asset metadata (asset_id = @asset_id).'), array('@asset_id' => $asset_id,), WATCHDOG_ERROR);
    _media_mediamosa_asset_delete($asset_id, $user_id);
    return FALSE;
  }
}

/**
 * Create mediafile.
 *
 * @param $asset_id
 * @param $user_id
 *
 * @return $mediafile_id
 */
function _media_mediamosa_mediafile_create($asset_id, $user_id) {
  // Check the parameters.
  if (empty($asset_id) || empty($user_id)) {
    return FALSE;
  }


  $data = array(
    'asset_id' => $asset_id,
    'user_id' => $user_id,
  );

  $response = _media_mediamosa_connector('mediafile/create', array('method' => 'POST', 'data' => $data,));

  if (!$response) {
    drupal_set_message(t('Error creating mediafile.'), 'error');
    watchdog('media_mediamosa', t('Error creating mediafile (asset_id = @asset_id).'), array('@asset_id' => $asset_id,), WATCHDOG_ERROR);
    _media_mediamosa_asset_delete($asset_id, $user_id);
    return FALSE;
  }

  $mediafile_id = (string) $response->xml->items->item->mediafile_id;

  return $mediafile_id;
}

/**
 * Create uploadticket.
 *
 * @param $asset_id
 * @param $mediafile_id
 * @param $user_id
 *
 * @param $ticket
 */
function _media_mediamosa_uploadticket_create($asset_id, $mediafile_id, $user_id) {
  // Check the parameters.
  if (empty($asset_id) || empty($mediafile_id) || empty($user_id)) {
    return FALSE;
  }


  $data = array(
    'user_id' => $user_id,
  );

  $response = _media_mediamosa_connector("mediafile/$mediafile_id/uploadticket/create", array('method' => 'POST', 'data' => $data,));

  if (!$response) {
    drupal_set_message(t('Error creating uploadticket for mediafile.'), 'error');
    watchdog('media_mediamosa', t('Error creating uploadticket for mediafile (mediafile_id = @mediafile_id).'), array('@mediafile_id' => $mediafile_id,), WATCHDOG_ERROR);
    _media_mediamosa_asset_delete($asset_id, $user_id);
    return FALSE;
  }

  $xml = mediamosa_connector::response2array($response);
  $ticket = $xml['items']['item'][0];

  return $ticket;
}

/**
 * Asset delete.
 *
 * @param $asset_id
 * @param $user_id
 */
function _media_mediamosa_asset_delete($asset_id, $user_id) {
  // Check the parameters.
  if (empty($asset_id) || empty($user_id)) {
    return FALSE;
  }


  $data = array(
    'user_id' => $user_id,
    'delete' => 'cascade',
  );

  $response = _media_mediamosa_connector("asset/$asset_id/delete", array('method' => 'POST', 'data' => $data,));

  if (!$response) {
    drupal_set_message(t('Error delete asset.'), 'error');
    watchdog('media_mediamosa', t('Error delete asset (asset_id = @asset_id).'), array('@asset_id' => $asset_id,), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Get asset.
 *
 * @param $asset_id
 *
 * @return $asset
 */
function _media_mediamosa_asset_get($asset_id) {
  // Check the parameters.
  if (empty($asset_id)) {
    return FALSE;
  }


  $response = _media_mediamosa_connector('asset/' . $asset_id);

  if (!$response) {
    drupal_set_message(t('Error getting asset.'), 'error');
    watchdog('media_mediamosa', t('Error getting asset (asset_id = @asset_id).'), array('@asset_id' => $asset_id,), WATCHDOG_ERROR);
    return FALSE;
  }

  $xml = mediamosa_connector::response2array($response);

  if (empty($xml['items']['item'][0])) {
    return FALSE;
  }

  $asset = $xml['items']['item'][0];

  return $asset;
}

/**
 * Search assets.
 *
 * @param $parameters
 *
 * @return $response
 */
function _media_mediamosa_assets_search($parameters) {
  $parameters += array(
    'limit' => 10,
  );

  $parameters_imploded = array();
  foreach ($parameters as $key => $parameter) {
    $parameters_imploded[] = $key . '=' . $parameter;
  }

  $response = _media_mediamosa_connector('asset?' . implode('&', $parameters_imploded));

  if (!$response) {
    drupal_set_message(t('Error getting asset.'), 'error');
    watchdog('media_mediamosa', t('Error getting asset (asset_id = @asset_id).'), array('@asset_id' => $asset_id,), WATCHDOG_ERROR);
    return FALSE;
  }

  $xml = mediamosa_connector::response2array($response);

  return $xml;
}

/**
 * Mediafile delete.
 *
 * @param $mediafile_id
 * @param $user_id
 */
function _media_mediamosa_mediafile_delete($mediafile_id, $user_id) {
  // Check the parameters.
  if (empty($mediafile_id) || empty($user_id)) {
    return FALSE;
  }


  $data = array(
    'user_id' => $user_id,
    'delete' => 'cascade',
  );

  $response = _media_mediamosa_connector("mediafile/" . $mediafile_id . "/delete", array('method' => 'POST', 'data' => $data,));

  if (!$response) {
    drupal_set_message(t('Error delete mediafile.'), 'error');
    watchdog('media_mediamosa', t('Error delete mediafile (mediafile_id = @mediafile_id).'), array('@mediafile_id' => $mediafile_id,), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Mediafile update.
 *
 * @param $mediafile_id
 * @param $user_id
 */
function _media_mediamosa_mediafile_update($mediafile_id, $user_id) {
  // Check the parameters.
  if (empty($mediafile_id) || empty($user_id)) {
    return FALSE;
  }


  $data = array(
    'user_id' => $user_id,
    'transcode_inherits_acl' => 'TRUE',
  );

  $response = _media_mediamosa_connector("mediafile/" . $mediafile_id, array('method' => 'POST', 'data' => $data,));

  if (!$response) {
    drupal_set_message(t('Error updating mediafile.'), 'error');
    watchdog('media_mediamosa', t('Error updating mediafile (mediafile_id = @mediafile_id).'), array('@mediafile_id' => $mediafile_id,), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Get mediafile.
 *
 * @param $mediafile_id
 *
 * @return $mediafile
 */
function _media_mediamosa_mediafile_get($mediafile_id) {
  // Check the parameters.
  if (empty($mediafile_id)) {
    return FALSE;
  }

  $response = _media_mediamosa_connector('mediafile/' . $mediafile_id);

  if (!$response) {
    drupal_set_message(t('Error getting mediafile.'), 'error');
    watchdog('media_mediamosa', t('Error getting mediafile (mediafile_id = @mediafile_id).'), array('@mediafile_id' => $mediafile_id,), WATCHDOG_ERROR);
    return FALSE;
  }

  $xml = mediamosa_connector::response2array($response);

  if (empty($xml['items']['item'][0])) {
    return FALSE;
  }

  $mediafile = $xml['items']['item'][0];

  return $mediafile;
}

/**
 * Get the video object.
 */
function _media_mediamosa_video_object_get($asset_id, $mediafile_id, $user_id, $autoplay, $width, $height, $response = 'plain') {
  // Check the parameters.
  if (empty($asset_id) || empty($mediafile_id) || empty($user_id)) {
    return FALSE;
  }

  $response = _media_mediamosa_connector('asset/' . $asset_id . '/play?mediafile_id=' . $mediafile_id . '&user_id=' . $user_id . '&autostart=' . ($autoplay ? 'TRUE' : 'FALSE') . '&width=' . $width . '&height=' . $height . '&response=' . $response);

  if (!$response) {
    drupal_set_message(t('Error getting video object.'), 'error');
    watchdog('media_mediamosa', t('Error getting video object (asset_id = @asset_id, mediafile_id = @mediafile_id).'), array('@asset_id' => $asset_id, '@mediafile_id' => $mediafile_id,), WATCHDOG_ERROR);
    return FALSE;
  }

  $xml = mediamosa_connector::response2array($response);

  $output = $xml['items']['item'][0]['output'];

  return $output;
}

/**
 * Still create.
 *
 * @param $mediafile_id
 * @param $user_id
 * @param $still_parameters
 *   An array with all the still parameters.
 */
function _media_mediamosa_still_create($mediafile_id, $user_id, $still_parameters) {
  // Check the parameters.
  if (empty($mediafile_id) || empty($user_id)) {
    return FALSE;
  }

  $still_parameters['user_id'] = $user_id;

  $response = _media_mediamosa_connector("mediafile/$mediafile_id/still/create", array('method' => 'POST', 'data' => $still_parameters,));

  if (!$response) {
    drupal_set_message(t('Error create still.'), 'error');
    watchdog('media_mediamosa', t('Error create still (mediafile_id = @mediafile_id).'), array('@mediafile_id' => $mediafile_id,), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Set default still.
 */
function _media_mediamosa_still_default($asset_id, $mediafile_id, $still_id, $user_id) {
  // Check the parameters.
  if (empty($asset_id) || empty($mediafile_id) || empty($still_id) || empty($user_id)) {
    return FALSE;
  }

  $data = array(
    'mediafile_id' => $mediafile_id,
    'still_id' => $still_id,
    'user_id' => $user_id,
  );

  $response = _media_mediamosa_connector("asset/$asset_id/still/default", array('method' => 'POST', 'data' => $data,));

  if (!$response) {
    drupal_set_message(t('Error set still default.'), 'error');
    watchdog('media_mediamosa', t('Error set still default (asset_id = @asset_id, mediafile_id = @mediafile_id, still_id = @still_id).'), array('@asset_id' => $asset_id, '@mediafile_id' => $mediafile_id, '@still_id' => $still_id,), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Delete still.
 */
function _media_mediamosa_still_delete($asset_id, $mediafile_id, $still_id, $user_id) {
  // Check the parameters.
  if (empty($asset_id) || empty($mediafile_id) || empty($still_id) || empty($user_id)) {
    return FALSE;
  }

  $data = array(
    'mediafile_id' => $mediafile_id,
    'still_id' => $still_id,
    'user_id' => $user_id,
  );

  $response = _media_mediamosa_connector("asset/$asset_id/still/delete", array('method' => 'POST', 'data' => $data,));

  if (!$response) {
    drupal_set_message(t('Error delete still.'), 'error');
    watchdog('media_mediamosa', t('Error delete still (asset_id = @asset_id, mediafile_id = @mediafile_id, still_id = @still_id).'), array('@asset_id' => $asset_id, '@mediafile_id' => $mediafile_id, '@still_id' => $still_id,), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Get transcode profiles.
 *
 * @return $profiles
 */
function _media_mediamosa_transcode_profiles_get() {

  $response = _media_mediamosa_connector('transcode/profile');

  if (!$response) {
    drupal_set_message(t('Error getting transcode profile.'), 'error');
    watchdog('media_mediamosa', t('Error getting transcode profile.'), array(), WATCHDOG_ERROR);
    return FALSE;
  }

  $xml = mediamosa_connector::response2array($response);

  $profiles = array();
  foreach ($xml['items']['item'] as $item) {
    $profiles[$item['profile_id']] = $item;
  }

  return $profiles;
}

/**
 * Create transcoded mediafile.
 */
function _media_mediamosa_transcode_create($mediafile_id, $user_id, $profile_id) {
  // Check the parameters.
  if (empty($mediafile_id) || empty($profile_id) || empty($user_id)) {
    return FALSE;
  }

  $data = array(
    'user_id' => $user_id,
    'profile_id' => $profile_id,
  );

  $response = _media_mediamosa_connector("mediafile/$mediafile_id/transcode", array('method' => 'POST', 'data' => $data,));

  if (!$response) {
    drupal_set_message(t('Error transcode mediafile.'), 'error');
    watchdog('media_mediamosa', t('Error transcode mediafile (mediafile_id = @mediafile_id, profile_id = @profile_id).'), array('@mediafile_id' => $mediafile_id, '@profile_id' => $profile_id,), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Get job for mediafile.
 *
 * @param $mediafile_id
 *
 * @return $jobs
 */
function _media_mediamosa_job_get($mediafile_id, $user_id = NULL) {
  // Check the parameters.
  if (empty($mediafile_id)) {
    return FALSE;
  }

  if (!$user_id) {
    $mediafile = _media_mediamosa_mediafile_get($mediafile_id);
    $user_id = (string) $mediafile['owner_id'];
  }

  $response = _media_mediamosa_connector('mediafile/' . $mediafile_id . '/joblist?user_id=' . $user_id);

  if (!$response) {
    drupal_set_message(t('Error getting jobs.'), 'error');
    watchdog('media_mediamosa', t('Error getting jobs (mediafile_id = @mediafile_id).'), array('@mediafile_id' => $mediafile_id,), WATCHDOG_ERROR);
    return FALSE;
  }

  $xml = mediamosa_connector::response2array($response);

  $jobs = array();
  if ($xml['items']['item']) {
    foreach ($xml['items']['item'] as $item) {
      $jobs[$item['id']] = $item;
    }
  }

  return $jobs;
}

/**
 * Get the currently changed assets.
 *
 * @param $time_last
 *
 * @return $assets
 */
function _media_mediamosa_current_assets_get($time_last) {
  // Check the parameters.
  if (empty($time_last) && $time_last !== 0) {
    return FALSE;
  }

  // Do request.
  // We avoid use of date('c') because of possible UTC additions, which not handled by CQL correctly.
  // Example: videotimestampmodified>2011-01-27T11:00:00
  //
  // We used 'changed' parameter here before, but at the moment 'videotimestampmodified' is much better option.
  // Despite of its name, it reflects cases when we change only metadata.
  $request = sprintf('asset?cql=videotimestampmodified>%s&limit=%d', gmdate('Y-m-d\TH:i:s', $time_last), MEDIA_MEDIAMOSA_ASSET_CQL_LIMIT);
  $response = _media_mediamosa_connector($request);

  if (!$response) {
    drupal_set_message(t('Error getting assets.'), 'error');
    watchdog('media_mediamosa', t('Error getting assets (time_last = @time_last).'), array('@time_last' => $time_last,), WATCHDOG_ERROR);
    return FALSE;
  }

  $xml = mediamosa_connector::response2array($response);

  $assets = !empty($xml['items']['item']) ? $xml['items']['item'] : array();

  return $assets;
}



/**
 * Simple function to correctly convert simplexml to array.
 *
 * @param object $xml.
 */
function _media_mediamosa_simplexml2array($xml) {
  $xml_copy = $xml;
  if (is_object($xml)) {
    if (get_class($xml) == 'SimpleXMLElement') {

      foreach($xml->attributes() as $key => $value) {
        if ($value) {
          $attributes[$key] = (string) $value;
        }
      }
      $xml = get_object_vars($xml);
    }
  }

  if (is_array($xml)) {
    if (empty($xml)) {
      return (string) $xml_copy;
    }

    $result = array();

    foreach($xml as $key => $value) {
      $result[$key] = _media_mediamosa_simplexml2array($value);
    }

    return $result;
  }

  return (string) $xml;
}

/**
 * Basicly the same as simplexml2array, with the exception that it will fix
 * the problem with single and multi items where a single item will end up
 * in a non-indexed array and multi items will.
 *
 * @param object $xml.
 */
function _media_mediamosa_responsexml2array($xml) {

  // Convert to nice array.
  $a_xml = _media_mediamosa_simplexml2array($xml);

  if (!isset($a_xml['items']['item'])) {
    return $a_xml;
  }

  // Fix the problem with 1 or 2+ items in array.
  $a_tmp = isset($a_xml['items']['item'][0]) ? $a_xml['items']['item'] : array(0 => $a_xml['items']['item']);
  unset($a_xml['items']);
  $a_xml['items']['item'] = $a_tmp;

  return $a_xml;
}

/**
 * Retrieve access limitations of mediafile.
 *
 * @param $mediafile_id
 * @param array $mediafile_id
 *   array with domains and realms. Function is responsible to determine correct type for each element.
 */
function _media_mediamosa_get_access_control($mediafile_id, $user_id) {
  // Check the parameters.
  if (empty($mediafile_id) || empty($user_id)) {
    return FALSE;
  }

  $request_options = array(
    'method' => 'GET',
    'user_id' => $user_id,
  );

  $response = _media_mediamosa_connector('mediafile/' . rawurlencode($mediafile_id) . '/acl', $request_options);

  if (!$response) {
    drupal_set_message(t('Error getting ACL.'), 'error');
    watchdog('media_mediamosa', t('Error getting ACL (mediafile_id = @mediafile_id).'), array('@mediafile_id' => $mediafile_id,), WATCHDOG_ERROR);
    return FALSE;
  }

  if (!mediamosa_connector::check_result($response)) {
    drupal_set_message(t('Error getting ACL.'), 'error');
    $request_result_description = empty($response->xml->header->request_result_description) ? '' : (string)$response->xml->header->request_result_description;

    watchdog('media_mediamosa', t('Error getting ACL (mediafile_id = @mediafile_id). @request_result_description'),
      array('@mediafile_id' => $mediafile_id, '@request_result_description' => $request_result_description), WATCHDOG_ERROR);
    return FALSE;
  }

  $xml = mediamosa_connector::response2array($response);

  return empty($xml['items']['item']) ? array() : $xml['items']['item'];
}

/**
 * Apply access limitations to mediafile.
 *
 * @param $mediafile_id
 * @param $user_id
 * @param array $access_entries
 *   array with domains and realms. Function is responsible to determine correct type for each element.
 */
function _media_mediamosa_set_access_control($mediafile_id, $user_id, $access_entries) {
  // Check the parameters.
  if (empty($mediafile_id) || empty($user_id)) {
    return FALSE;
  }

  $request_options = array(
    'method' => 'POST',
    'user_id' => $user_id,
    'fatal' => FALSE,
    'data' => array(
      'replace' => 'TRUE',
    ),
  );

  foreach ($access_entries as $entry) {
    if (_media_mediamosa_is_realm($entry)) {
      $request_options['data']['aut_realm'][] = $entry;
    }
    else {
      $request_options['data']['aut_domain'][] = $entry;
    }
  }

  $response = _media_mediamosa_connector('mediafile/' . rawurlencode($mediafile_id)
    . '/acl' . ($access_entries ? '' : '/delete'), $request_options);

  if (!$response) {
    drupal_set_message(t('Error setting ACL.'), 'error');
    watchdog('media_mediamosa', t('Error setting ACL (mediafile_id = @mediafile_id).'), array('@mediafile_id' => $mediafile_id,), WATCHDOG_ERROR);
    return FALSE;
  }

  return TRUE;
}

/**
 * Update reference id.
 *
 * @param $node_id
 * @param $asset_id
 *
 */
function _media_mediamosa_reference_id_update($asset_id, $node_id, $user_id) {
  // Check the parameters.
  if (empty($asset_id) || empty($node_id) || empty($user_id)) {
    return FALSE;
  }

  $data = array(
    'group_id' => $node_id,
    'user_id' => $user_id,
    'is_app_admin' => 'TRUE',
  );
  $response = _media_mediamosa_connector('asset/' . $asset_id, array('method' => 'POST', 'data' => $data,));
  if (!$response) {
    drupal_set_message(t('Error updating reference id.'), 'error');
    watchdog('media_mediamosa', t('Error updating reference id.'), WATCHDOG_ERROR);
    return FALSE;
  }
  return TRUE;

}

/**
 * Get asset by reference id.
 *
 * @param $node_id
 *
 * @return $asset
 */
function _media_mediamosa_asset_reference_get($node_id) {
  // Check the parameters.
  if (empty($node_id)) {
    return FALSE;
  }

  $data = array(
    'group_id' => $node_id,
    'group_id_match' => 'exact',
  );
  $response = _media_mediamosa_connector('asset/', array('method' => 'GET', 'data' => $data,));

  if (!$response) {
    drupal_set_message(t('Error getting asset.'), 'error');
    watchdog('media_mediamosa', t('Error getting asset (reference_id = @node_id).'), array('@node_id' => $node_id,), WATCHDOG_ERROR);
    return FALSE;
  }

  $xml = mediamosa_connector::response2array($response);
  $asset = $xml['items']['item'][0];

  return $asset;
}
