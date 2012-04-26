<?php
/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * Any admin or user related code and hooks.
 */

/**
 * The main configuration form for MediaMosa Construction Kit.
 *
 * Uses the hook_mediamosa_ck_configuration_collect() to collect settings.
 *
 * @return
 *  An array representing the form definition.
 *
 * @ingroup forms
 * @see media_mediamosa_configuration_form_validate()
 * @see media_mediamosa_configuration_form_submit()
 */
function mediamosa_ck_configuration_form() {
  // Build the settings form and enrich it with our settings collect hook.
  return system_settings_form(module_invoke_all('mediamosa_ck_configuration_collect'));
}

/**
 * Form validation handler for main configuration form for MediaMosa
 * Construction Kit.
 *
 * @see media_mediamosa_configuration_form()
 */
function mediamosa_ck_configuration_form_validate($form, &$form_state) {

  // Call our (optional) validation hooks.
  module_invoke_all('mediamosa_ck_configuration_collect_validate', $form, $form_state);
}

/**
 * Form submission handler for the main configuration form for MediaMosa
 * Construction Kit.
 *
 * @see media_mediamosa_configuration_form()
 */
function mediamosa_ck_configuration_form_submit($form, &$form_state) {

  // Call our (optional) submit hooks.
  module_invoke_all('mediamosa_ck_configuration_collect_submit', $form, $form_state);
}
