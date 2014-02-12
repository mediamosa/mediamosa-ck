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
    if (!empty($this->value)) {
      $this->query->options['owner_id'] = $this->value;
    }
    unset($this->query->options['owned_id']);
    parent::_cql_exact();
  }

  /**
   * Provide a simple checkbox.
   */
  public function value_form(&$form, &$form_state) {
    $options = array(1 => 'Only my assets');
    $form['value'] = array(
      '#type' => 'select',
      '#title' => 'My assets',
      '#options' => $options,
      '#default_value' => '',
    );
  }
}
