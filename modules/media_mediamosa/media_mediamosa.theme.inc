<?php
/**
 * @file
 * All theming functions.
 */

/**
 * Format the link.
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
  $parts = mediamosa_ck::fileuri2parts($file_managed['uri']);

  // Return link to media.
  return l(check_plain($parts['filename']), 'file/' . $file_managed['fid']);
}

/**
 * Preview for Styles UI.
 */
function theme_media_mediamosa_preview_style($variables) {
  // @todo: fixme.
  $variables['uri'] = 'FIXME: Theme';
  $variables['field_type'] = 'file';
  $variables['object'] = file_uri_to_object($variables['uri']);
  return theme('styles', $variables);
}

/**
 * Video player theme.
 */
function theme_media_mediamosa_video($variables) {
  // To render, asset_id must be present.
  if (empty($variables['element']['#uri'])) {
    return;
  }

  $values = mediamosa_ck::fileuri2parts($variables['element']['#uri']);

  // Get options if set.
  $options = array();
  if (isset($variables['element']['#options']) && !empty($variables['element']['#options'])) {
    $options = $variables['element']['#options'];
  }

  // What mediafile to play?
  $mediafile_id_to_play = empty($values['mediafile_id']) ? NULL : $values['mediafile_id'];

  // Take user_id of current watcher.
  $user_id = mediamosa_ck::session_user_id();

  // Determine admin user.
  if (user_access('access administration pages')) {
    $options['is_app_admin'] = TRUE;
  }

  // Response.
  $options['response'] = 'object';
  $options['profile_id'] = MediaMosaCkConnectorWrapper::get_default_transcode_profile();

  // Both not empty.
  if (!empty($variables['element']['#width']) && !empty($variables['element']['#height'])) {
    $options['width'] = $variables['element']['#width'];
    $options['height'] = $variables['element']['#height'];
    $options['autostart'] = $variables['element']['#autoplay'] ? 'TRUE' : 'FALSE';
  }

  try {
    $options['fatal'] = TRUE;
    $response = MediaMosaCkConnectorWrapper::get_play_link($values['asset_id'], $mediafile_id_to_play, $user_id, $options);
  }
  catch (Exception $e) {
    $response = theme(
      'mediamosa_ck_views_player_image',
      array(
        'width' => $options['width'],
        'height' => $options['height'],
        'error_code' => $e->getCode(),
        'asset_id' => $values['asset_id'],
        'mediafile_id' => $mediafile_id_to_play,
      )
    );
  }
  return $response;
}
