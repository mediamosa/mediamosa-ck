<?php
/**
 * @file
 * All related theme hook functions.
 */

/**
 * @param array $variables
 *   An associative array;
 *
 *   - 'title'
 *     The title of the section.
 *   - 'description
 *     Description shown on top.
 *
 * @return string
 *   The HTML.
 */
function theme_mediamosa_ck_cs_log($variables) {

  // Build as form.
  $form = array();

  $variables['pager normal bottom'] = TRUE;

  $form['log'] = array(
    '#title' => $variables['title'],
    '#description' => $variables['description'],
    '#theme' => 'mediamosa_ck_popup',
    '#markup' => theme('mediamosa_ck_table', $variables),
  );

  return drupal_render($form);
}
