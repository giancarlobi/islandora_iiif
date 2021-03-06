<?php

/**
 * @file
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_iiif_admin_settings_form(array $form, array &$form_state) {
  form_load_include($form_state, 'inc', 'islandora_iiif', 'includes/admin.form');
  $get_default_value = function ($name, $default) use (&$form_state) {
    return isset($form_state['values'][$name]) ? $form_state['values'][$name] : variable_get($name, $default);
  };
  $solr_enabled = module_exists('islandora_solr');
  $djatoka_url = $get_default_value('islandora_iiif_djatoka_url', 'http://localhost:8080/adore-djatoka/resolver');
  $djatoka_available_message = islandora_iiif_admin_settings_form_djatoka_available_message($djatoka_url);
  $form = array(

      'islandora_iiif_iiif_url' => array(
        '#type' => 'textfield',
        '#title' => t('IIIF Image Server Base URL'),
        '#default_value' => variable_get('islandora_iiif_iiif_url', 'iiif/2'),
        '#description' => t('The location of the IIIF Image Server.'),
      ),


      'islandora_iiif_tile_url_prefix' => array(
       '#type' => 'textfield',
       '#title' => t('Tile source URL prefix'),
       '#description' => t('Prefix to compose tile source URL (prefix + PID + suffix).'),
       '#default_value' => variable_get('islandora_iiif_tile_url_prefix', 'http://127.0.0.1:8080/fedora/objects/'),
      ),

     'islandora_iiif_tile_url_suffix' => array(
       '#type' => 'textfield',
       '#title' => t('Tile source URL suffix'),
       '#description' => t('Suffix to compose tile source URL (prefix + PID + suffix).'),
       '#default_value' => variable_get('islandora_iiif_tile_url_suffix', '/datastreams/OBJ/content'),
      ),

      'islandora_iiif_solr_num_of_results' => array(
	    '#type' => 'textfield',
	    '#title' => t('Collection max results per page'),
	    '#size' => 5,
	    '#description' => t('Max number of results to show per page, if more pager is enabled'),
	    '#default_value' => variable_get('islandora_iiif_solr_num_of_results', '20'),
      ),


  // @TODO need to define how we are going to fetch web annotations. Probably add an API entry and a hookable callback selector

  );
  return system_settings_form($form);
}

/**
 * Ajax callback for the djatoka url textfield.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The element to render as part the ajax callback.
 */
function islandora_iiif_admin_settings_form_djatoka_ajax_callback(array $form, array &$form_state) {
  return $form['islandora_iiif_djatoka_url'];
}

/**
 * Gets a message to display if the djatoka is accessible or not.
 *
 * @param string $djatoka_url
 *   The url to an djatoka instance.
 *
 * @return string
 *   A message in html detailing if the djatoka is accessible.
 */
function islandora_iiif_admin_settings_form_djatoka_available_message($djatoka_url) {
  $djatoka_url = trim($djatoka_url, '/');
  $djatoka_no_resolver = preg_replace('/resolver$/', '', $djatoka_url);

  $file_headers = @get_headers($djatoka_no_resolver);
  $djatoka_availible = strstr($file_headers[0], '200') !== FALSE;
  if ($djatoka_availible) {
    $image = theme_image(array('path' => 'misc/watchdog-ok.png', 'attributes' => array()));
    $message = t('Adore-djatoka url is valid.');
  }
  else {
    $image = theme_image(array('path' => 'misc/watchdog-error.png', 'attributes' => array()));
    $message = t('Unable to locate djatoka endpoint at @path', array('@path' => $djatoka_url));
  }
  return $image . $message;
}
