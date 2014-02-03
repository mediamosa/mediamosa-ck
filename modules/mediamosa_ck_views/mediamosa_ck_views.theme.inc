<?php
/**
 * @file
 * Theme related functions.
 */

/**
 * Show listing of mediafiles.
 *
 * The theme used to generate a listing of mediafiles in the views using
 * mediamosa_ck_views_field_text_mediafiles.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_asset_mediafiles($variables) {
  // Empty.
  $output = array();

  if (empty($variables['mediafiles'])) {
    return t('There are no other formats available');
  }

  // Loop.
  foreach ($variables['mediafiles'] as $mediafile) {
    if (empty($mediafile['metadata']['container_type'])) {
      // Not analysed.
      continue;
    }

    // Build the link.
    $path = strtr($variables['asset_path'], array('[asset_id]' => $mediafile['asset_id'], '[mediafile_id]' => $mediafile['mediafile_id']));

    // Get filename.
    $output[] = '<li>';
    $output[] = empty($mediafile['filename']) ? t('No filename present') : (l(trim($mediafile['filename']), $path));
    $output[] = '</li>';
  }

  return empty($output) ? '' : '<ul>' . implode('', $output) . '</ul>';
}

/**
 * Show listing of downloads.
 *
 * The theme used to generate a listing of downloads in the views using
 * mediamosa_ck_views_field_text_download.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_asset_download($variables) {
  $output = array();

  if (empty($variables['download'])) {
    return t('No downloads available');
  }

  foreach ($variables['download'] as $mediafile) {

    $filename = empty($mediafile['filename']) ? t('No filename present') : trim($mediafile['filename']);
    if (isset($mediafile['metadata']) && (isset($mediafile['metadata']['filesize']))) {
      $filename .= " (" . format_size($mediafile['metadata']['filesize']) . ")";
    }

    // Build the link.
    $path = strtr($variables['asset_path'], array('[asset_id]' => $mediafile['asset_id'], '[mediafile_id]' => $mediafile['mediafile_id']));
    $output[] = '<li>';
    $output[] = l($filename, $path . '/download', array('attributes' => array('class' => 'download')));
    $output[] = '</li>';
  }

  return empty($output) ? '' : '<ul>' . implode('', $output) . '</ul>';
}

/**
 * Show list of collections.
 *
 * The theme used to generate a listing of collections in the views using
 * mediamosa_ck_views_field_text_collections.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_asset_collections($variables) {
  // Empty.
  $output = array();

  if (empty($variables['collections'])) {
    return;
  }

  // Loop.
  foreach ($variables['collections'] as $collection) {
    $output[] = '<li>' . l($collection['title'], strtr($variables['collection_path'], array('[coll_id]' => $collection['coll_id']))) . '</li>';
  }

  return empty($output) ? '' : '<ul>' . implode('', $output) . '</ul>';
}

/**
 * Shows listing of asset metadata.
 *
 * The theme used to generate a listing of metadata in the views using
 * mediamosa_ck_views_field_text_metadata.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_asset_metadata($variables) {
  // Empty.
  $rows = array();

  ksort($variables['metadata']);

  foreach ($variables['metadata'] as $name => $value) {
    $name = drupal_ucfirst(str_replace('_', ' ', $name));

    if (is_array($value)) {
      $value = implode("\n", $value);
    }

    $rows[] = array('name' => $name, 'value' => (empty($value) ? ' ' : nl2br(check_plain($value))));
  }

  return theme('table', array('rows' => $rows));
}

/**
 * Shows listing of technical metadata.
 *
 * The theme used to generate a listing of technical metadata in the views using
 * mediamosa_ck_views_field_text_technical_metadata.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_asset_technical_metadata($variables) {
  // Empty.
  $rows = array();
  ksort($variables['technical_metadata']);

  foreach ($variables['technical_metadata'] as $name => $value) {
    $name = drupal_ucfirst(str_replace('_', ' ', $name));
    $rows[] = array(
      'name' => $name,
      'value' => (empty($value) ? ' ' : t($value, array(), array('context' => ''))),
    );
  }

  return theme('table', array('rows' => $rows));
}

/**
 * The theme used to generate links to the edit/delete pages for collections.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_collection_actions($variables) {
  // Empty.
  $output = array();

  // @todo ; owner check.
  $output[] = '<li>' . l(t('edit'), strtr($variables['action_path_edit'], array('[coll_id]' => $variables['coll_id']))) . '</li>';
  $output[] = '<li>' . l(t('delete'), strtr($variables['action_path_delete'], array('[coll_id]' => $variables['coll_id']))) . '</li>';

  return empty($output) ? t('No actions') : '<ul>' . implode('', $output) . '</ul>';
}

/**
 * The theme used to generate links to the edit/delete pages for assets.
 *
 * @param array $variables
 *   Data used for the theme.
 */
function theme_mediamosa_ck_views_theme_asset_actions($variables) {
  // Empty.
  $output = array();

  // Check for ownership on asset.
  if (mediamosa_sb::is_owner_asset($variables['asset_id'])) {
    $output[] = '<li>' . l(t('edit'), strtr($variables['action_path_edit'], array('[asset_id]' => $variables['asset_id']))) . '</li>';
    $output[] = '<li>' . l(t('delete'), strtr($variables['action_path_delete'], array('[asset_id]' => $variables['asset_id']))) . '</li>';
  }

  return empty($output) ? t('No actions') : '<ul>' . implode('', $output) . '</ul>';
}

/**
 * Create themed link to owner.
 *
 * The theme will generate a link to the user using given data. When uid is 0 or
 * missing, then only the contents of user name is printed.
 *
 * @param array $variables
 *   See theme definition for possible attributes.
 */
function theme_mediamosa_ck_views_theme_owner($variables) {
  if (empty($variables['uid'])) {
    return empty($variables['name']) ? t('<no name>') : check_plain($variables['name']);
  }

  return l($variables['name'], 'user/' . $variables['uid']);
}

/**
 * Theme's a mediamosa player object.
 *
 * @param array $variables
 *   string with a player object from MediaMosa.
 */
function theme_mediamosa_ck_views_player($variables) {
  global $base_url;

  if (strpos($variables['response'], 'type="audio/')) {
    $path = $base_url . '/' . drupal_get_path('module', 'mediamosa_ck') . '/modules/mediamosa_ck_views/images/';
    $audioimage = $path . 'audio.png';
    $variables['response'] = str_replace('poster=""', 'poster="' . $audioimage . '"', $variables['response']);
  }
  // Full screen code for iframes etc., @todo: make 100% possible.
  $variables['response'] = str_replace('width="942" height="942"', 'width="100%" height=""', $variables['response']);

  return $variables['response'];
}


/**
 * Theme's a player with an error.
 *
 * @param array $variables
 *   array with player object info.
 */
function theme_mediamosa_ck_views_player_image($variables) {

  $path = drupal_get_path('module', 'mediamosa_ck') . '/modules/mediamosa_ck_views/images/';
  if ($variables['error_code'] == '1800') {
    $image = $path .  'notgranted.png';
  }
  else {
    $image = $path .  'notfound.png';
  }
  $response = theme('image',
    array(
      'path' => $image,
      'alt' => 'Mediafile not found',
      'attributes' => array(
        'width' => $variables['width'],
        'height' => $variables['height'],
        'class' => 'mediafile-notfound',
      ),
    )
  );
  return $response;
}
