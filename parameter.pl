#!/usr/bin/perl

use Getopt::Long;

my $usage = "Usage: $0 [-q (query from stdin)] [-c <configfile> ] -s <sourcefile> -d <destfile> \n";
my %opts;
if(!GetOptions(\%opts, "q","c=s","s=s","d=s")) { 
        print "GetOptions error\n";
        print "$usage";
        exit(-1);
}

my $query = $opts{"q"};
my $sourcefile = $opts{"s"};
my $destfile = $opts{"d"};
my $cfgfile = $opts{"c"};


if( !$sourcefile ||!destfile ){
    print "$usage\n";
    exit(-1);
}
if( !$cfgfile ){
    $cfgfile = "setup.cfg";
}	
if( ! -e $cfgfile ){
    print "setup.cfg doesn't exist !\n";
    exit(-1);	
}	

#read setup.cfg
my $cfg_ref = &read_cfg( $cfgfile );
if( !defined( $cfg_ref->{'variable'}->{'OMC_ID'} ) ){
    print "\nInput OMC_ID : ";
    while(1){
    	my $val = <>  ;
    	if( $val =~ /^\s*$/ ){
            print "\nInput OMC_ID : ";
    	    next ; 
    	}
    	chomp( $val );
    	$cfg_ref->{'variable'}->{'OMC_ID'} = $val ;
    	last;
    }
}	 

#query from stdin if -q is used
if( defined( $query)  ){
    &query_cfg( $cfgfile ) ;
}

#substitude parameter in $sourcefile
&substitute_parameter( $sourcefile, $destfile) ;

sub substitute_parameter{ 
    $sourcefile = shift ;
    $destfile = shift ;
    
    open (SRC,"$sourcefile") or die "Can't read file: $sourcefile\n";
    open (DEST,">$destfile") or die "Can't read file: $destfile\n";
    
    my $line ;
    while( $line = <SRC> ){
        my $hash_vars = $cfg_ref->{'variable'};
        foreach my $name ( keys(%$hash_vars) ){
            my $value = $hash_vars ->{$name};
            $line =~  s/\$${name}/$value/ig ;
            $line =~  s/\$\{${name}\}/$value/ig ;
        }
        
        print DEST "$line";    	
    }
    
    close( SRC );
    close( DEST);
}

sub query_cfg{

    my $cfgfile = shift ;
    my $hash_vars = $cfg_ref->{'variable'};
    foreach my $name ( sort keys(%$hash_vars) ){
    	my $val = $hash_vars->{$name};
        print "\nInput $name [$val]:";
        $val=<>;
        if( $val =~ /^\s*$/ ){
            next ;
        }
        chomp( $val );
        $hash_vars->{$name} = $val ;   
    }	    	
    
    #update cfgfile
    open (CFG,">$cfgfile") or die "Can't read file: $cfgfile\n";
    print CFG "[variable]\n";
    foreach my $name ( sort keys(%$hash_vars) ){
    	my $val = $hash_vars->{$name};
        print CFG "$name=$val\n";
    }	    	
    close(CFG);
	
}	

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
            $attr_name = uc($1) ;
            $attr_value = $2 ;
            $attr_ref->{$entity}->{$attr_name} = $attr_value     ;
            next;
        }       
    }
    close(CFG);
    
    return $attr_ref  ;
}
