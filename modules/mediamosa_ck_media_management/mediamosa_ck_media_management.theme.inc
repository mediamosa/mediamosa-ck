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
function theme_mediamosa_ck_mm_log($variables) {

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

/**
 * Shows an name-value-pair combination.
 */
function theme_mediamosa_ck_mm_name_value_pair($variables) {

  $value = $variables['default_value'];
  $value = $variables['check_plain'] ? check_plain($value) : check_plain($value);
  return '<div class="mediamosa-ck-cs-name">' . check_plain($variables['title']) . '</div><div class="mediamosa-ck-cs-value">' . $value . '</div>';
}
