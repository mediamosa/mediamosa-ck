<?php
/**
 * @file
 * Views field owned.
 */

class mediamosa_ck_views_field_owned extends views_handler_field {

  /**
   * Options_definition.
   */
  public function option_definition() {
    $options = parent::option_definition();

    $options['owned'] = array('default' => '');
    $options['user_empty_name'] = array('default' => '-');

    return $options;
  }

  /**
   * Options_form.
   */
  public function options_form(&$form, &$form_state) {
    $form['owned'] = array(
      '#type' => 'checkbox',
      '#title' => t('Owned by logged in user.'),
    );

    parent::options_form($form, $form_state);
  }

  /**
   * Not used and overruled.
   */
  public function pre_render(&$values) {
  }

  /**
   * Added owned field.
   */
  public function add_additional_fields($fields = NULL) {
    foreach ($fields as $field) {
      $this->aliases[$field] = $this->query->add_field($this->table, $field);
    }
  }

  public function click_sort($order) {
    $this->query->add_orderby($this->table, $this->real_field, $order);
  }

  /**
   * Not sure if this gets used.
   */
  public function query() {
    $this->field_alias = $this->query->add_field($this->table, $this->real_field);

    // Add in additional fields.
    if (!empty($this->additional_fields)) {
      $this->add_additional_fields($this->additional_fields);
    }
  }

  /**
   * Render a link to the user.
   */
  public function render($values) {
    global $user;

    // We use the owner theme to theme the link.
    return theme('mediamosa_ck_views_theme_owner', array('uid' => $user->uid, 'name' => $user->name));
  }
}
