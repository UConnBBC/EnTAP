.. _NCBI Taxonomy: https://www.ncbi.nlm.nih.gov/taxonomy
.. |libs_dir| replace:: /libs
.. |entap_dir| replace:: /EnTAP
.. |src_dir| replace:: /src
.. |config_file| replace:: entap_config.txt
.. |bin_dir| replace:: /bin
.. |data_dir| replace:: /databases
.. |tax_file| replace:: download_tax.pl
.. |graph_file| replace:: entap_graphing.py
.. |go_term| replace:: go_term.entp
.. |tax_bin| replace:: ncbi_tax_bin.entp
.. |tax_data| replace:: ncbi_tax.entp


Basic Usage
============

*EnTAP* has two stages of execution, :ref:`configuration<config-label>` and :ref:`run<run-label>`. Configuration is generally ran first (and may only need to be ran once) to setup databases while run is reserved for the main annotation pipeline and can be ran multiple times after initial configuration.

.. _config-label:

Configuration
-------------
Configuration is the first stage of EnTAP that will download and configure the necessary databases for full functionality. This stage may only need to be ran once (unless you'd like to configure more databases for DIAMOND). 


Folder Hierarchy
^^^^^^^^^^^^^^^^^

Before continuing, I'll explain the folder hierarchy that will be assumed throughout the rest of this tutorial. From this point on, the 'main' or 'exe' directory of EnTAP will refer to the setup as downloaded from the GitLab page. 

This |entap_dir| directory contains:

    * |entap_dir| |libs_dir| 
    * |entap_dir| |src_dir|
    * |entap_dir| |bin_dir| (created during configuration)
    * |entap_dir| |data_dir| (created during configuration)

In addition to some other files/directories. This 'main' folder can be changed with the - - exe flag (discussed later)

Recognition of EnTAP databases, src files, and execution accompanying pipeline software rely on this directory hierarchy. However, some files/directories can be changed from the default with the  |config_file| file. 

.. warning:: Renaming folders/files within the main EnTAP directory can cause execution issues

The |config_file| file mentioned above has the following defaults:

    * diamond_exe_path=/EnTAP/libs/diamond-0.8.31/bin/diamond
    * rsem_exe_path=/EnTAP/libs/RSEM-1.3.0 (this is a path to the directory)
    * genemarkst_exe_path=/EnTAP//libs/gmst_linux_64/gmst.pl
    * eggnog_exe_path=/EnTAP/libs/eggnog-mapper/emapper.py
    * eggnog_download_exe=/EnTAP/libs/eggnog-mapper/
    * eggnog_dmnd_database=/EnTAP/libs/eggnog-mapper/data/eggnog.db


These can be changed to whichever path you would prefer. If something is globally installed, just put a space " " after the '='. EnTAP will recognize these paths first and they will override defaults. 

The following paths cannot be changed from the defaults within the 'main' folder:

    * |entap_dir| / |config_file|
    * |entap_dir| |src_dir| / |graph_file|
    * |entap_dir| |src_dir| / |tax_file|
    * |entap_dir| |bin_dir| / |tax_bin| (downloaded during config)
    * |entap_dir| |bin_dir| / |go_term| (downloaded during config)
    * |entap_dir| |data_dir| / |tax_data| (downloaded during config)

This EnTAP directory will be automatically detected (from the EnTAP exe), however the - -exe flag can be used to change this. As long it is pointed to a directory with the above files/paths there will be no execution issues. 

Usage
^^^^^

Databases in traditional .fasta (or similar) format must be configured to run with EnTAP for faster similarity searching. This can be done with any database you have previously downloaded and will be configured and sent to the /bin folder within the main EnTAP directory. 

To run configuration with a sample database, the command is as follows:

.. code-block:: bash

    EnTAP --config -d path/to/database

This stage must be done at least once prior to :ref:`running<run-label>`. Once the database is configured, you need not do it again unless you updated your original database or plan on configuring several others.


.. note:: If you already have DIAMOND (.dmnd) configured databases, you can skip the configuration of that database. Although, due to other *EnTAP* database downloading (taxonomy and ontology), configuration must still be ran at least once without any flags.

Configuration can be ran without formatting a database as follows:

.. code-block:: bash

    EnTAP --config


.. note:: This is the only stage that requires connection to the Internet.

Flags:
^^^^^^^^^^^^^^^^^^^^^

Required Flags:

    * The only required flag is **- -config**. Although in order to run the full *EnTAP* pipeline, you must have a .dmnd configured database.


Optional Flags:

    * -d : Specify any number of databases you would like to configure for EnTAP

    * -exe: Change 'main' directory
    * -database-out: Change output directory for formatted DIAMOND databases



Memory Usage:
^^^^^^^^^^^^^^

Memory usage will vary depending on the number of databases you would like configured. Although, EnTAP will download several other databases as well:

* Gene Ontology References: 6Mb
* NCBI Taxonomy: 400Mb

.. _run-label:

Run
-------------
The run stage of *EnTAP* is the main annotation pipeline. After configuration is ran at least once, this can be ran continually without requiring configuration to be ran again (unless more databases will be configured). 

Input Files:
^^^^^^^^^^^^
Required:

* .FASTA formatted transcriptome file (either protein or nucleotide)
* .dmnd (DIAMOND) indexed databases, which can be formatted in the :ref:`configuration<config-label>` stage. Up to 4 can be chosen


Optional:

* .BAM/.SAM alignment file. If left unspecified expression filtering will not be performed. 

Sample Run:
^^^^^^^^^^^

A specific run flag (**runP/runN**) must be used:

* runP: Indicates protein input transcripts. Selection of this option will skip the frame selection portion of the pipeline.
* runN: Indicates nucleotide input transcripts. Selection of this option will cause frame selection to be ran. 


An example run with a nucleotide transcriptome:

.. code-block:: bash

    enTAP --runN -i path/to/transcriptome.fasta -d path/to/database.dmnd -d path/to/database2.dmnd -a path/to/alignment.sam


With the above command, the entire *enTAP* pipeline will be ran. However, it is entirely possible to skip frame selection by inputting protein transcripts (- -runP) or skip expression filtering by excluding an alignment file. 


Flags:
^^^^^^^^^^^^^^^^^^^^^

Required Flags:

* (- -runP/- -runN)
    * Specification of input transcriptome file. runP for protein (skip frame selection) or runN for nucleotide (frame selection will be ran)

* (-i/- -input)
    * Path to the transcriptome file (either nucleotide or protein)

* (-d/- -database)
    * Specify up to 4 DIAMOND indexed (.dmnd) databases to run similarity search against

Optional Flags:

* (-a/- -align)
    * Path to alignment file (either SAM or BAM format)
    * **Note:** Ignoring this flag will skip expression filtering

* (- -contam)
    * Specify :ref:`contaminant<tax-label>` level of filtering
    * Multiple contaminants can be selected through repeated flags

* (- -species)
    * This flag will allow for taxonomic 'favoring' of hits that are closer to your target species or lineage. Any lineage can be used as referenced by the NCBI Taxonomic database, such as genus, phylum, or species.
    * Format **must** replace all spaces with underscores ('_') as follows: "- -species homo_sapiens" or "- -species primates"

* (- -tag)
    * Specify output folder labelling.
    * Default: /outfiles

* (- - fpkm)
    * Specify FPKM cutoff for expression filtering
    * Default: 0.5

* (- - coverage)
    * Specify minimum query coverage for similarity searching
    * Default: 50%

* (- - overwrite)
    * All previously ran files will be overwritten if the same - -tag flag is used
    * Without this flag *enTAP* will :ref:`recognize<over-label>` previous runs

* (- - state)
    * Precise control over execution stages. This flag allows for certain parts to be ran while skipping others. 


.. _tax-label:

Taxonomic Contaminant Filtering
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Taxonomic contaminant filtering (as well as taxonomic favoring) is based upon the `NCBI Taxonomy`_ database. 

.. _over-label:

Picking Up Where You Left Off
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


.. _state-label:
State Control
^^^^^^^^^^^^^^
