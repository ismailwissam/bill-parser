#!/usr/bin/perl

use Data::Dumper;
use Getopt::Long;

my $usage = "Usage: $0 -cfg <config file> -tab <table name> -attr <attribute name> \n";
my %opts;
if(!GetOptions(\%opts, "cfg=s","tab=s","attr=s")) { 
        print "GetOptions error\n";
        print "$usage";
        exit(-1);
}

my $cfg_file   = $opts{"cfg"};
my $tab_name   = lc( $opts{"tab"} );
my $attr_name  = lc( $opts{"attr"} );

if( !$cfg_file || !$tab_name || !$attr_name ){
    print "$usage\n";
    exit(-1);
}

my $cfg_ref = &read_cfg( $cfg_file );

my $ret  = $cfg_ref->{$tab_name}->{$attr_name} ;

print "$ret\n";


sub read_cfg {
    my $file_name = shift ;

    my $attr_ref ;
    my ($entity ,$attr_name ,$attr_value) ;
    open (CFG,"$file_name") or die "Can't read file: $file_name\n";
    while( $line = <CFG> ){
        
        if( $line =~ /^\s*\[\s*(\S+)\s*\]\s*$/ ){
            $entity = lc($1); 
            next ;              
        }   
      
        if(  $line =~ /^\s*#/ ){
             next;
        } 
    
        if( $line =~ /^\s*(\S+)\s*=\s*(\S+)\s*$/ ){
            $attr_name = lc($1) ;
            $attr_value = $2 ;
            $attr_ref->{$entity}->{$attr_name} = $attr_value     ;
            next;
        }       
    }
    close(CFG);
    
    return $attr_ref  ;
}

