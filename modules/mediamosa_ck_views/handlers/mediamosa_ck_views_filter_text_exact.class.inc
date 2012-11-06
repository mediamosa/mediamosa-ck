<?php
/**
 * @file
 * Views filter text exact.
 */

class mediamosa_ck_views_filter_text_exact extends mediamosa_ck_views_filter {

  /**
   * Add input to filter data.
   */
  public function query() {
    parent::_cql_exact();
  }

  /**
   * Provide a simple textfield for equality.
   */
  public function value_form(&$form, &$form_state) {
    $form['value'] = array(
      '#type' => 'textfield',
      '#title' => t($this->definition['title']),
      '#default_value' => $this->value,
    );
  }
}
