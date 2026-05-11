# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Fluvel'
copyright = '2026, Fabien Bessy'
author = 'Fabien Bessy'
release = '0.1'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

# object_description_options = [
#     ("cpp:.*", dict(include_object_type_in_xref_tooltip=False)),
# ]

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}

extensions = [
    "breathe",
    "myst_parser",
    "sphinx_design",
]

templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "pydata_sphinx_theme"

html_static_path = ['_static']

html_css_files = [
    'fluvel.css',
]

breathe_projects = {
    "Fluvel": "../../xml"
}

breathe_default_project = "Fluvel"

html_logo = "_static/fluvel.svg"
