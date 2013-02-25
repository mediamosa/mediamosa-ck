<?php
/**
 * @file
 * All admin related functions are put here.
 */

/**
 * Show the upload page.
 */
function _mediamosa_ck_upload_page() {
  // Get the upload form.
  $variables['left'] = drupal_render(drupal_get_form('mediamosa_ck_upload_form'));

  // Include the upload listing form.
  $variables['right'] = drupal_render(drupal_get_form('mediamosa_ck_upload_listing_form'));

  return theme('mediamosa_ck_two_column_page', $variables);
}

/**
 * The basic upload form.
 */
function mediamosa_ck_upload_form($form, $form_state) {
  $form['mediamosa_ck_upload'] = array(
    '#type' => 'mediamosa_ck_upload',
    '#title' => 'MediaMosa Bulk Upload',
    '#upload_validators' => array('file_validate_extensions' => mediamosa_ck_upload::get_default_upload_validators()),
  );

  return $form;
}

/**
 * Upload listing form.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 *
 * @return array
 *   The upload listing.
 */
function mediamosa_ck_upload_listing_form($form, $form_state) {
  $form['mediamosa_ck_upload_listing'] = array(
    '#prefix' => '<div id="mediamosa-ck-upload-listing">',
    '#suffix' => '</div>',
    '#markup' => '',
  );
  $form['mediamosa_ck_upload_listing_refresh'] = array(
    '#type' => 'button',
    '#value' => t('Refresh'),
    '#ajax' => array(
      'callback' => '_mediamosa_ck_content_supplier_upload_listing_refresh_ajax',
      'wrapper' => 'mediamosa-ck-upload-listing',
      'method' => 'replace',
      'effect' => 'fade',
      'speed' => 'fast',
      'event' => 'click',
      'progress' => array('message' => ''),
    ),
    '#attached' => array(
      'js' => array(
        drupal_get_path('module', 'mediamosa_ck_upload') . '/mediamosa_ck_upload.js',
      ),
    ),
  );

  return $form;
}

/**
 * Returns the contents for upload listing.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 *
 * @return string
 *   The HTML to copy into wrapper.
 */
function _mediamosa_ck_content_supplier_upload_listing_refresh_ajax($form, $form_state) {

  // Search for assets.
  $assets = mediamosa_ck::search_asset();

  if ($assets === FALSE) {
    $content = t('Search failed, check MediaMosa connection.');
  }
  else {
    $variables = array(
      'header' => array(
        t('title'),
        t('date'),
      ),
      'rows' => array(),
      'empty' => t('No assets uploaded.'),
    );

    foreach ($assets as $asset_id => $asset) {
      $variables['rows'][] = array(
        array(
          'data' => $asset['title'],
        ),
        array(
          'data' => $asset['videotimestampmodified'],
        ),
      );
    }

    // Use theme to create table.
    $content = theme('mediamosa_ck_table', $variables);
  }

  return '<div id="mediamosa-ck-upload-listing">' . $content . '</div>';
}
