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
 *   This include processes mediamosa.com media files for use by emfield.module.
 */

/**
 * Implementation of hook emvideo_PROVIDER_info().
 * Return the information about a specific provider.
 *
 *  @return
 *    An associative array with the following information:
 *      'provider' => The machine name, should be same as the provider filename.
 *      'name' => The provider's human readable name.
 *      'url' => The URL to the provider's main page.
 *      'settings_description' => This will be displayed above the provider
 *        information on the administration page.
 *      'supported_features' => An array of cells to be passed to a table
 *        on the administration page, under the headers of 'Feature',
 *        'Supported', and 'Notes'.
 */
function emvideo_mediamosa_info() {
  $features = array(
    array(t('Autoplay'), t('Yes'), t('You may coustomize the player to enable or disable the autoplay function.')),
    array(t('RSS Attachment'), t('Yes'), ''),
    array(t('Thumbnails'), t('Yes'), t('')),
    array(t('Duration'), t('Yes'), ''),
    array(t('Link / URL'), t('Yes'), ''),
    array(t('Embedded code'), t('Yes'), ''),
    array(t('Full screen mode'), t('Yes'), t('You may customize the player to enable or disable full screen playback. Full screen mode is enabled by default.')),
    array(t('Use JW FLV Media Player'), t('Yes'), t("You may opt to use the !flvplayer to play mediamosa videos if it's installed on your server.", array('!flvplayer' => l(t('JW FLV Media Player'), 'http://www.longtailvideo.com/players/jw-flv-player/')))),
  );
  return array(
    'provider' => 'mediamosa',
    'name' => t('MediaMosa'),
    'url' => MEDIA_MEDIAMOSA_MAIN_URL,
    'settings_description' => t('These settings specifically affect videos displayed from <a href="@mediamosa" target="_blank">MediaMosa</a>. You can learn more about its <a href="@api" target="_blank">API</a> here.', array('@mediamosa' => MEDIA_MEDIAMOSA_MAIN_URL, '@api' => MEDIA_MEDIAMOSA_API_URL)),
    'supported_features' => $features,
  );
}

/**
 * Implementation of hook emvideo_PROVIDER_settings().
 * This should return a subform to be added to the emvideo_settings() admin settings page.
 * Note that a form field will already be provided, at $form['PROVIDER'] (such as $form['mediamosa']).
 * So if you want specific provider settings within that field, you can add the elements to that form field.
 */
/*
function emvideo_mediamosa_settings() {
  module_load_include('inc', 'media_mediamosa', 'includes/media_mediamosa.admin');
  return media_mediamosa_admin_form();
}
*/

/**
 *  Implement of hook emvideo_PROVIDER_data_version().
 */
function emvideo_mediamosa_data_version() {
  return MEDIA_MEDIAMOSA_DATA_VERSION;
}

/**
 * Implementation of hook emfield_PROVIDER_rss().
 *
 * Embeds the video in the RSS feed.
 */
function emvideo_mediamosa_rss($item, $teaser = NULL) {
  if (empty($item['value'])) {
    return FALSE;
  }

  $asset_id = $item['value'];
  $mediafile = _media_mediamosa_get_mediafile_original($asset_id);
  $mediafile_id = $mediafile['mediafile_id'];

  $file = array();

  $mediafile = _media_mediamosa_mediafile_get($mediafile_id);

  $mime_type = $mediafile['metadata']['mime_type'];

  $filepath = emvideo_mediamosa_embedded_link($mediafile_id);

  $file['filepath'] = ($filepath ? $filepath : variable_get('mediamosa_connector_url', ''));
  $file['filesize'] = emvideo_mediamosa_duration($item);
  $file['filemime'] = $mime_type;
  // Just the $item value is used.
  $file['thumbnail']['filepath'] = emvideo_mediamosa_thumbnail($field, $item, $formatter, $node, $width, $height);

  return $file;
}

/**
 * Implementation of hook emvideo_PROVIDER_extract().
 * Parse the URL or embed code provided by an editor.
 *
 *  @param $embed
 *    The raw URL or embed code pasted into the text field by the editor.
 *  @param $field
 *    The field data for the emvideo, emaudio, etc.
 *  @return
 *    If the hook returns a non-empty string, then it is assumed to have been
 *    parsed and matched by this provider. If the hook returns an array of
 *    strings, then each string is assumed to be a regex pattern, and will
 *    be checked for a match in turn. Otherwise, it is assumed there is no
 *    match.
 */
function emvideo_mediamosa_extract($embed, $field) {
  if (preg_match('/^[A-Za-z0-9]+$/', $embed)) {
    return $embed;
  }
}

/**
 * Implementation of hook emvideo_PROVIDER_thumbnail().
 * Returns the external url for a thumbnail of a specific video.
 *
 *  @param $field
 *    the field of the requesting node
 *  @param $item
 *    the actual content of the field from the requesting node
 *  @param $formatter
 *    the formatter, eg. video_thumbnail
 *  @param $node
 *    the node object
 *  @param $width
 *    the width to display the thumbnail
 *  @param $height
 *    the height to display the thumbnail
 *  @param $options
 *    other options
 *
 *  @return
 *    a URL pointing to the thumbnail
 */
function emvideo_mediamosa_thumbnail($field, $item, $formatter, $node, $width, $height, $options = array()) {
  if (empty($item['value'])) {
    return FALSE;
  }

  $asset_id = $item['value'];
  $asset = _media_mediamosa_asset_get($asset_id);
  $thumbnail = (string) $asset['vpx_still_url'];

  return $thumbnail;
}

/**
 * Implementation of hook emvideo_PROVIDER_video().
 * Displays the full/normal-sized video.
 *
 *  @param $code
 *    the video code for the video to embed
 *  @param $width
 *    the width to display the video
 *  @param $height
 *    the height to display the video
 *  @param $field
 *    the field info from the requesting node
 *  @param $item
 *    the actual content from the field
 *  @param $node
 *    the actual node
 *  @param $autoplay
 *    the autoplay field, boolean
 *  @param $options
 *    other options
 *
 *  @return
 *    the html of the embedded video
 */
function emvideo_mediamosa_video($code, $width, $height, $field, $item, $node, $autoplay, $options = array()) {
  $asset_id = $code;
  $mediafile = _media_mediamosa_get_mediafile_original($asset_id);
  $mediafile_id = $mediafile['mediafile_id'];

  $node_type = $node->type;

  return _media_mediamosa_generate_video_output($mediafile_id, $node_type, $autoplay, $width, $height);
}

/**
 * Implementation of hook emvideo_PROVIDER_video().
 * Displays the preview-sized video, commonly for the teaser.
 *
 *  @param $code
 *    the video code for the video to embed
 *  @param $width
 *    the width to display the video
 *  @param $height
 *    the height to display the video
 *  @param $field
 *    the field info from the requesting node
 *  @param $item
 *    the actual content from the field
 *  @param $node
 *    the actual node
 *  @param $autoplay
 *    the autoplay field, boolean
 *  @param $options
 *    other options
 *
 *  @return
 *    the html of the embedded video
 */
function emvideo_mediamosa_preview($code, $width, $height, $field, $item, $node, $autoplay, $options = array()) {
  return emvideo_mediamosa_video($code, $width, $height, $field, $item, $node, $autoplay, $options);
}

/**
 * Implementation of hook emvideo_PROVIDER_embedded_link().
 * Returns a link to view the video at the provider's site.
 *
 *  @param $video_code
 *    the string containing the video to watch
 *
 *  @return
 *    a string containing the URL to view the video at the original provider's site
 */
function emvideo_mediamosa_embedded_link($video_code) {
  $asset_id = $video_code;
  $mediafile = _media_mediamosa_get_mediafile_original($asset_id);
  $mediafile_id = $mediafile['mediafile_id'];
  $user_id = $mediafile['owner_id'];

  // Get the basic data.
  $mediafile = _media_mediamosa_mediafile_get($mediafile_id);


  // Get the video plain response.
  $autoplay = 'TRUE';
  $width = NULL;
  $height = NULL;
  $output = _media_mediamosa_video_object_get($asset_id, $mediafile_id, $user_id, $autoplay, $width, $height);

  return $output;
}

/**
 * Implementation of hook emvideo_PROVIDER_duration().
 * Returns the duration of the video in seconds.
 *
 *  @param $item
 *    The video item itself, which needs the $data array.
 *
 *  @return
 *    The duration of the video in seconds.
 */
function emvideo_mediamosa_duration($item) {
  if (empty($item['value'])) {
    return FALSE;
  }

  $asset_id = $item['value'];
  $mediafile = _media_mediamosa_get_mediafile_original($asset_id);
  $mediafile_id = $mediafile['mediafile_id'];

  // Get the basic data.
  $mediafile = _media_mediamosa_mediafile_get($mediafile_id);

  if (empty($mediafile['metadata']['file_duration'])) {
    return;
  }

  $duration = _media_mediamosa_parse_duration((string) $mediafile['metadata']['file_duration']);

  return $duration;
}

/**
 * Implementation of hook_emvideo_PROVIDER_content_generate().
 */
/*
function emvideo_mediamosa_content_generate() {
  return array(
    'http://mediamosa.org/...',
    'http://mediamosa.org/...',
    'http://mediamosa.org/...',
  );
}
 */

/**
 * Implementation of hook emvideo_PROVIDER_form().
 */
/*
function emvideo_mediamosa_form($field, $item) {
}
 */

/**
 * This passes on the original implementation of CCK's hook_widget_settings.
 */
/*
function media_mediamosa_emfield_widget_settings_extra($op, $widget) {
}
 */

/**
 * This passes on the original implementation of CCK's hook_widget.
 */
/*
function media_mediamosa_emfield_widget_extra($form, $form_state, $field, $items, $delta, $module) {
}
 */
