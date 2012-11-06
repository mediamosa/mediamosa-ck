<?php
/**
 * @file
 * Filter for collection selection in an exposed form.
 */

class mediamosa_ck_views_filter_owner extends mediamosa_ck_views_filter {

  /**
   * We need to convert from Drupal name to Drupal mail address.
   *
   * @param string $value
   *   The value to change.
   *
   * @return string
   *   Return the converted value.
   */
  public function _convert_value($value) {

    // Get the user.
    $user = mediamosa_ck::lookup_user_with_name($value);

    // If user can not be found, then we return the original.
    return empty($user) ? $value : $user->mail;
  }

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
      '#autocomplete_path' => 'user/autocomplete',
    );
  }
}
