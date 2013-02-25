<?php
/**
 * @file
 * Theme views implementations.
 */

/**
 * Display the view as a media browser.
 *
 * @param array $vars
 *   Variables to preprocess for the mediamosa media browser
 */
function template_preprocess_media_mediamosa_views_view_media_browser(&$vars) {

  // Default media preprocess with customisation.
  module_load_include('inc', 'media', 'includes/media.browser');

  // Load file objects for each View result.
  $fids = array();
  foreach ($vars['rows'] as $index => $row) {
    $fids[$index] = $row->fid;
  }
  $files = file_load_multiple($fids);

  // Render the preview for each file.
  foreach ($vars['rows'] as $index => $row) {

    $file = $files[$row->fid];
    // Add url/preview to the file object.
    media_browser_build_media_item($file);
    $vars['rows'][$index] = $file;

    // Add fields to the preview.
    if (!empty($vars['view']->style_plugin->rendered_fields[$index])) {
      $file->preview .= '<div class="mediamosabrowser-field-wrapper">';
    }
    foreach ($vars['view']->style_plugin->rendered_fields[$index] as $key => $field) {
      $file->preview .= '<div class="mediamosabrowser-field-' . $key . '">' . $field . '</div>';
    }
    if (!empty($vars['view']->style_plugin->rendered_fields[$index])) {
      $file->preview .= '</div>';
    }
    $vars['rows'][$index]->preview = l($file->preview, 'media/browser', array(
      'html' => TRUE,
      'attributes' => array(
        'data-fid' => $row->fid,
      ),
      'query' => array('render' => 'media-popup', 'fid' => $row->fid))
    );
  }

  // Add the files to JS so that they are accessible inside the browser.
  drupal_add_js(array('media' => array('files' => array_values($files))), 'setting');

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
