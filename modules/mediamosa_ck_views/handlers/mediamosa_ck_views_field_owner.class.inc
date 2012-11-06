<?php
/**
 * @file
 * Views field owner.
 */

class mediamosa_ck_views_field_owner extends views_handler_field {
  public function option_definition() {
    $options = parent::option_definition();

    $options['user_empty_name'] = array('default' => '-');

    return $options;
  }

  public function options_form(&$form, &$form_state) {
    $form['user_empty_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Default user name'),
      '#description' => t(''),
      '#default_value' => isset($this->options['user_empty_name']) ? $this->options['user_empty_name'] : '',
    );

    parent::options_form($form, $form_state);
  }

  // The following methods express strong SQLisms in the base field handler
  // class. Override and blank them out.
  public function pre_render(&$values) {
  }

  public function add_additional_fields($fields = NULL) {
    foreach ($fields as $field) {
      $this->aliases[$field] = $this->query->add_field($this->table, $field);
    }
  }

  public function click_sort($order) {
    $this->query->add_orderby($this->table, $this->real_field, $order);
  }

  public function query() {
    $this->field_alias = $this->query->add_field($this->table, $this->real_field);

    // Add in additional fields.
    if (!empty($this->additional_fields)) {
      $this->add_additional_fields($this->additional_fields);
    }
  }

  public function render($values) {
    // Must exist.
    assert(isset($values->owner_id));

    // Lookup the owner.
    $user = mediamosa_ck::lookup_user_with_mail($values->owner_id);

    if (empty($user)) {
      $user = new stdClass();
      $user->uid = 0;
      $user->name = $this->options['user_empty_name'];
    }

    return theme('mediamosa_ck_views_theme_owner', array('uid' => $user->uid, 'name' => $user->name));
  }
}
