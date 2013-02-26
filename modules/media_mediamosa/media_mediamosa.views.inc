<?php
/**
 * @file
 * Theme views implementations.
 */

/**
 * Implements hook_views_plugins().
 *
 * @return array
 *   View plugins.
 */
function media_mediamosa_views_plugins() {
  $plugins = array();

  // Display plugin.
  $plugins['display']['media_mediamosa_browser'] = array(
    'title' => t('Media Mediamosa browser'),
    'help' => t('Display as a tab in the media browser.'),
    'handler' => 'media_mediamosa_views_plugin_display_media_browser',
    'theme' => 'media_mediamosa_views_view_media_browser',
    'base' => array('mediamosa_asset_search'),
    'use ajax' => TRUE,
    'use pager' => TRUE,
    'accept attachments' => TRUE,
  );

  // Style plugin.
  $plugins['style']['media_mediamosa_browser'] = array(
    'title' => t('Media Mediamosa browser'),
    'help' => t('Displays rows as an HTML list.'),
    'handler' => 'media_mediamosa_views_plugin_style_media_browser',
    'theme' => 'media_mediamosa_views_view_media_browser',
    'base' => array('mediamosa_asset_search'),
    'uses row plugin' => FALSE,
    'uses row class' => FALSE,
    'uses options' => FALSE,
    'uses fields' => TRUE,
    'type' => 'normal',
    'help topic' => 'style-media-browser',
  );
  return $plugins;
}

/**
 * Display the view as a media browser.
 *
 * @param array $vars
 *   Variables to preprocess for the mediamosa media browser
 */
function template_preprocess_media_mediamosa_views_view_media_browser(&$vars) {

  // Default media preprocess with customisation.
  module_load_include('inc', 'media', 'includes/media.browser');

  // Collect asset_ids.
  $asset_ids = array();
  foreach ($vars['rows'] as $row) {
    $asset_ids[] = $row->asset_id;
  }

  // Contains $filemanaged_assets[$asset_id] => array('fid' => $fid)
  $filemanaged_assets = mediamosa_ck::get_filemanaged_from_assets($asset_ids);

  // Load file objects for each View result.
  $fids = array();
  foreach ($filemanaged_assets as $asset_id => $filemanaged_asset) {
    $fids[$asset_id] = $filemanaged_asset['fid'];
  }
  $files = file_load_multiple($fids);

  // Render the preview for each file.
  foreach ($vars['rows'] as $index => $row) {
    if (isset($fids[$row->asset_id])) {
      $file = $files[$fids[$row->asset_id]];

      // Add url/preview to the file object.
      media_browser_build_media_item($file);

      $vars['rows'][$index] = $file;
      $vars['rows'][$index]->preview = $file->preview;
    }
    else {
      // Not found as mediafile.
      unset($vars['rows'][$index]);
    }
  }

  // Add the files to JS so that they are accessible inside the browser.
  drupal_add_js(array('media' => array('files' => array_values($files))), 'setting');

  // Add the browser parameters to the settings and that this display exists.
  drupal_add_js(array(
    'media' => array(
      'browser' => array(
        'params' => media_get_browser_params(),
        'views' => array(
          $vars['view']->name => array(
            $vars['view']->current_display,
          ),
        ),
      ),
    ),
  ), 'setting');

  // Add classes and wrappers from the style plugin.
  $handler  = $vars['view']->style_plugin;

  $class = explode(' ', $handler->options['class']);
  $class = array_map('drupal_clean_css_identifier', $class);

  $wrapper_class = explode(' ', $handler->options['wrapper_class']);
  $wrapper_class = array_map('drupal_clean_css_identifier', $wrapper_class);

  $vars['class'] = implode(' ', $class);
  $vars['wrapper_class'] = implode(' ', $wrapper_class);
  $vars['wrapper_prefix'] = '<div id="container" class="' . implode(' ', $wrapper_class) . '">';
  $vars['wrapper_suffix'] = '</div>';
  $vars['list_type_prefix'] = '<' . $handler->options['type'] . ' id="media-browser-library-list" class="' . implode(' ', $class) . '">';
  $vars['list_type_suffix'] = '</' . $handler->options['type'] . '>';

  // Run theming variables through a standard Views preprocess function.
  template_preprocess_views_view_unformatted($vars);

  // Add media browser javascript and CSS.
  drupal_add_js(drupal_get_path('module', 'media') . '/js/plugins/media.views.js');
}
