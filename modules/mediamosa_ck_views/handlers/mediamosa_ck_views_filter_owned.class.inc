<?php
/**
 * @file
 * Filter for 'owned' (only own assets) selection in an exposed form.
 */

class mediamosa_ck_views_filter_owned extends mediamosa_ck_views_filter {

  /**
   * Return mail address of current logged in user.
   */
  public function _convert_value($value) {

    if ($value) {
      return mediamosa_ck::session_user_id();
    }
  }

  /**
   * Add input to filter data.
   */
  public function query() {
    $this->field = 'owner_id';
    parent::_cql_exact();
  }

  /**
   * Provide a simple textfield for equality.
   */
  public function value_form(&$form, &$form_state) {
    $form['value'] = array(
      '#type' => 'checkbox',
      '#title' => 'My assets',
      '#default_value' => $this->value,
    );
  }
}
