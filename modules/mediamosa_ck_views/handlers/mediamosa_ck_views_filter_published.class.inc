<?php
/**
 * @file
 * Filter for 'published' (only own assets) selection in an exposed form.
 */

class mediamosa_ck_views_filter_published extends mediamosa_ck_views_filter {

  /**
   * Add input to filter data.
   */
  public function query() {
    $this->query->add_filter('published', $this->value);
    $this->field = 'published';
  }

  /**
   * Provide a simple checkbox.
   */
  public function value_form(&$form, &$form_state) {
    $options = array(
      'TRUE' => 'Only show published assets.',
      'ALL' => 'show all assets (including non published)',
      'FALSE' => 'Only show not published assets');

    $form['value'] = array(
      '#type' => 'select',
      '#title' => 'Published',
      '#options' => $options,
      '#default_value' => 'TRUE',
    );
  }
}
