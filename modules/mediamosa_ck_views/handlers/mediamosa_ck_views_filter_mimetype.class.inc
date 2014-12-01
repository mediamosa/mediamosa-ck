<?php
/**
 * @file
 * Filter for mimetypes selection in an exposed form. There are 2 classes here,
 * one for the original mediafile, and 1 filter on all mediafiles.
 */

class mediamosa_ck_views_filter_mimetype extends mediamosa_ck_views_filter {

  /**
   * No conversion needed.
   *
   * @param string $value
   *   The value to change.
   *
   * @return string
   *   Return the converted value.
   */
  public function _convert_value($value) {
    return ('^' . $value);
  }

  /**
   * Add input to filter data.
   */
  public function query() {
    parent::_cql();
  }

  /**
   * Provide a simple textfield for equality.
   */
  public function value_form(&$form, &$form_state) {
    $mime_types = array(
      'audio' => 'audio', 'image' => 'image', 'text' => 'text', 'video' => 'video');
    $form['value'] = array(
      '#type' => 'select',
      '#title' => $this->definition['title'],
      '#options' => array_merge(array('All' => t('- Any -')), $mime_types),
      '#default_value' => $this->value,
    );
  }
}

/**
 * Filter on the first part of mimetype of the original mediafile.
 */
class mediamosa_ck_views_filter_mimetype_original extends mediamosa_ck_views_filter {

  /**
   * No conversion needed.
   *
   * @param string $value
   *   The value to change.
   *
   * @return string
   *   Return the converted value.
   */
  public function _convert_value($value) {
    return ('^' . $value);
  }

  /**
   * Add input to filter data.
   */
  public function query() {
    parent::_cql();
  }

  /**
   * Provide a simple textfield for equality.
   */
  public function value_form(&$form, &$form_state) {
    $mime_types = array(
      'audio' => 'audio', 'image' => 'image', 'text' => 'text', 'video' => 'video');
    $form['value'] = array(
      '#type' => 'select',
      '#title' => $this->definition['title'],
      '#options' => array_merge(array('All' => t('- Any -')), $mime_types),
      '#default_value' => $this->value,
    );
  }
}
