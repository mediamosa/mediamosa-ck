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
    global $user;

    // First possible token-replace, makes [user:mail] possible.
    $username = token_replace($value, array('user' => $user));

    // Get the user.
    $lookup_name = mediamosa_ck::lookup_user_with_name($username);

    // If user can not be found, then we return the original.
    return !empty($lookup_name) ? $lookup_name : $username;
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
      '#title' => $this->definition['title'],
      '#default_value' => $this->value,
      '#autocomplete_path' => 'user/autocomplete',
    );
  }
}
