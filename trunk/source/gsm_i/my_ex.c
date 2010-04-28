
#include <ctpublic.h>
#include "insert.h"
#include "exutils.h"

CS_RETCODE CS_PUBLIC ex_execute_query(CS_CONNECTION *connection,CS_CHAR *cmdbuff,char ***buff, \
                                        int col,int begin,int num,int * o_num)
{
	CS_RETCODE	ret;
	CS_RETCODE	retcode;
	CS_COMMAND      *cmd=NULL;
	CS_INT		res_type;	/* result type from ct_results */


	ret=CS_SUCCEED;
	
	*o_num=0;
	
	/*
	** Allocate a command handle to send the compute query with
	*/
        if ( ct_cmd_alloc(connection, &cmd) != CS_SUCCEED )
        {
        	ret=CS_FAIL;
       		err_log("ex_execute_query: ct_cmd_alloc fail\n");
       		goto Exit_Pro;
        }

	/*
	** Define a language command that contains a compute clause.  SELECT
	** is a select statment defined in the header file.
	*/
	if( ct_command(cmd, CS_LANG_CMD, cmdbuff, CS_NULLTERM, CS_UNUSED) != CS_SUCCEED )
	{
		ret=CS_FAIL;
                err_log("ex_execute_query: ct_command fail\n");
                goto Exit_Pro;
	}

	/*
	** Send the command to the server 
	*/
	if( ct_send(cmd) != CS_SUCCEED )
	{
		ret=CS_FAIL;
                err_log("ex_execute_query: ct_send fail\n");
                goto Exit_Pro;
	}

	/*
	** Process the results.  Loop while ct_results() returns CS_SUCCEED.
	*/
	while ( (retcode = ct_results(cmd, &res_type)) == CS_SUCCEED )
	{
		switch ((int)res_type)
		{
		case CS_CMD_SUCCEED:
			/*
			** This means no rows were returned.  
			*/
			break;

		case CS_CMD_DONE:
			/*
			** This means we're done with one result set.
			*/
   			break;

		case CS_CMD_FAIL:
			/*
			** This means that the server encountered an error while
			** processing our command.
			*/
                	err_log("ex_execute_query: ct_results returned CMD_FAIL\n");
                	ret=CS_FAIL;
			break;

		case CS_ROW_RESULT:
			if( ex_fetch_query(cmd,buff,col,begin,num,o_num) != CS_SUCCEED )
			{
                	        err_log("ex_execute_query: ex_fetch_query fail\n");
				ret=CS_FAIL;
			}
			break;

		default:
			/*
			** We got an unexpected result type.
			*/
                	err_log("ex_execute_query: ct_results returned unexpected result type\n");
			ret=CS_FAIL;
		}
	}

	/*
	** We're done processing results. Let's check the
	** return value of ct_results() to see if everything
	** went ok.
	*/
	switch ( (int)retcode )
	{
		case CS_END_RESULTS:
			/*
			** Everything went fine.
			*/
			break;
		case CS_FAIL:
			/*
			** Something went wrong.
			*/
			ret=CS_FAIL;
                	err_log("ex_execute_query: ct_results fail\n");
		default:
			/*
			** We got an unexpected return value.
			*/
			ret=CS_FAIL;
                	err_log("ex_execute_query: ct_results returned unexpected result code\n");
	}

Exit_Pro:
	if(cmd!=NULL)
	{
		if( ct_cmd_drop(cmd) != CS_SUCCEED )
		{
			ret=CS_FAIL;
			err_log("ex_execute_query: cd_cmd_drop fail\n");
		}
	}
	
	return ret;
}

CS_RETCODE CS_PUBLIC ex_fetch_query(CS_COMMAND *cmd, char ***buff, int col, int begin, int num, int *o_num)
{
	CS_RETCODE	        ret;
	CS_RETCODE		retcode;
	CS_INT			num_cols;
	CS_INT			i;
	CS_INT			j;
	CS_INT			row_count = 0;
	CS_INT			rows_read;
	CS_DATAFMT		*datafmt = NULL;
	EX_COLUMN_DATA		*coldata = NULL;


	ret         = CS_SUCCEED;
	
	/*
	** Find out how many columns there are in this result set.
	*/
	if(ct_res_info(cmd, CS_NUMDATA, &num_cols, CS_UNUSED, NULL) != CS_SUCCEED)
	{
		err_log("ex_fetch_query: ct_res_info fail\n");
		ret=CS_FAIL;
		goto Exit_Pro;
	}

	/*
	** Make sure we have at least one column
	*/
	if (num_cols <= 0)
	{
		err_log("ex_fetch_query: ct_res_info returned zero columns\n");
		ret=CS_FAIL;
		goto Exit_Pro;
	}

	/*
	** Our program variable, called 'coldata', is an array of 
	** EX_COLUMN_DATA structures. Each array element represents
	** one column.  Each array element will re-used for each row.
	**
	** First, allocate memory for the data element to process.
	*/
	coldata = (EX_COLUMN_DATA *)malloc(num_cols * sizeof (EX_COLUMN_DATA));
	if (coldata == NULL)
	{
		err_log("ex_fetch_query: malloc fail\n");
		ret=CS_FAIL;
		goto Exit_Pro;
	}
	for (i = 0; i< num_cols; i++)
		coldata[i].value = NULL;
	
	datafmt = (CS_DATAFMT *)malloc(num_cols * sizeof (CS_DATAFMT));
	if (datafmt == NULL)
	{
		err_log("ex_fetch_query: malloc fail\n");
		ret=CS_FAIL;
		goto Exit_Pro;
	}
	
	for (i = 0; i < num_cols; i++)
	{
		/*
		** Get the column description.  ct_describe() fills the
		** datafmt parameter with a description of the column.
		*/
		if(ct_describe(cmd, (i + 1), &datafmt[i]) != CS_SUCCEED)
		{
			err_log("ex_fetch_query: ct_describe fail\n");
			ret=CS_FAIL;
			goto Exit_Pro;
		}

		/*
		** update the datafmt structure to indicate that we want the
		** results in a null terminated character string.
		**
		** First, update datafmt.maxlength to contain the maximum
		** possible length of the column. To do this, call
		** ex_display_len() to determine the number of bytes needed
		** for the character string representation, given the
		** datatype described above.  Add one for the null
		** termination character.
		*/
		datafmt[i].maxlength = ex_display_dlen(&datafmt[i]) + 1;
		
		/*
		** Set datatype and format to tell bind we want things
		** converted to null terminated strings
		*/
		datafmt[i].datatype = CS_CHAR_TYPE;
		datafmt[i].format   = CS_FMT_NULLTERM;
		datafmt[i].count    = 1;
		
		/*
		** Allocate memory for the column string
		*/
		coldata[i].value = (CS_CHAR *)malloc(datafmt[i].maxlength);
		if (coldata[i].value == NULL)
		{
			err_log("ex_fetch_query: malloc fail\n");
			ret=CS_FAIL;
			goto Exit_Pro;
		}

		/*
		** Now bind.
		*/
		if( ct_bind(cmd, (i + 1), &datafmt[i],coldata[i].value, &coldata[i].valuelen, \
		            (CS_SMALLINT *)&coldata[i].indicator) != CS_SUCCEED )
		{
			err_log("ex_fetch_query: ct_bind fail\n");
			ret=CS_FAIL;
			goto Exit_Pro;
		}
	}
	
	/*
	** Fetch the rows.  Loop while ct_fetch() returns CS_SUCCEED or 
	** CS_ROW_FAIL
	*/
	while (((retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, \
			&rows_read)) == CS_SUCCEED) || (retcode == CS_ROW_FAIL))
	{
		/*
		** Increment our row count by the number of rows just fetched.
		*/
		row_count = row_count + rows_read;

		/*
		** Check if we hit a recoverable error.
		*/
		if (retcode == CS_ROW_FAIL)
		{
			ct_cancel(NULL, cmd, CS_CANCEL_CURRENT);
			err_log("ex_fetch_query: ct_fetch CS_ROW_FAIL\n");
			ret=CS_FAIL;
			goto Exit_Pro;
		}
		if(row_count<begin)
			continue;
		
		if( row_count > (begin+num-1) )
		{
                        if(ct_cancel(NULL, cmd, CS_CANCEL_CURRENT) != CS_SUCCEED)
                        {
                        	err_log("ex_fetch_query: ct_cancel fail\n");
                        	ret=CS_FAIL;
                        	goto Exit_Pro;
                        }
                        retcode=CS_END_DATA;
                        break;
		}

		(*o_num)++;
		j=row_count-begin;
		
		/*
		** We have a row.  Loop through the columns displaying the
		** column values.
		*/
		for (i = 0; i < num_cols && i < col; i++)
		{	  
			strcpy(buff[j][i],coldata[i].value);
			/*
			** Display the column value for debug
			*/
			//fprintf(stdout, "%s#", coldata[i].value);
		} 
		//fprintf(stdout,"\n");
	}


	/*
	** We're done processing rows.  Let's check the final return
	** value of ct_fetch().
	*/
	switch ((int)retcode)
	{
		case CS_END_DATA:
			/*
			** Everything went fine.
			*/
			break;

		case CS_FAIL:
			/*
			** Something terrible happened.
			*/
			err_log("ex_fetch_query: ct_fetch fail\n");
			ret=CS_FAIL;
			break;

		default:
			/*
			** We got an unexpected return value.
			*/
			err_log("ex_fetch_query: ct_fetch returned an unexpected retcode\n");
			ret=CS_FAIL;
			break;

	}
		
Exit_Pro:
	if(coldata != NULL)
	{
		for (i = 0; i < num_cols; i++)
		{
			if(coldata[i].value != NULL)
				free(coldata[i].value);
		}
		free(coldata);
	}
	if(datafmt != NULL)  free(datafmt);
		
	return ret;
}


/*
 *  do_proc_after_insert()
 *                            return 0 success, 1 fail
 *
 *  调存储过程完成 insert detail, 提交接口表
 *
 */
int do_proc_after_insert(CS_CONNECTION *connection, int idb_num, char *par_cdr_switch, \
                          char *par_file_id, int *o_db_rec_num)
{
	int              ret;
	CS_COMMAND	*cmd=NULL;
	CS_RETCODE	 retcode;
	CS_INT		 res_type;	/* result type from ct_results */

	CS_SMALLINT	 msg_id;
	CS_DATAFMT	 datafmt;

	

	ret           = 0;
	*o_db_rec_num = 0;


        if ( ct_cmd_alloc(connection, &cmd) != CS_SUCCEED )
        {
        	ret=1;
       		err_log("do_proc_after_insert: ct_cmd_alloc fail\n");
       		goto Exit_Pro;
        }

	if( ct_command(cmd, CS_RPC_CMD, "p_base_insert_0", CS_NULLTERM, CS_NO_RECOMPILE) != CS_SUCCEED )
	{
		ret=1;
                err_log("do_proc_after_insert: ct_command fail\n");
                goto Exit_Pro;
	}

	memset(&datafmt, 0, sizeof (datafmt));
	strcpy(datafmt.name, "@in_num");
	datafmt.namelen = CS_NULLTERM;
	datafmt.datatype = CS_INT_TYPE;
	datafmt.maxlength = CS_UNUSED;
	datafmt.status = CS_INPUTVALUE;
	datafmt.locale = NULL;
	if ( ct_param(cmd, &datafmt, (CS_VOID *)&idb_num, CS_SIZEOF(CS_INT), CS_UNUSED) != CS_SUCCEED )
	{
		ret=1;
                err_log("do_proc_after_insert: ct_param 1 fail\n");
                goto Exit_Pro;
	}

	memset(&datafmt, 0, sizeof (datafmt));
	strcpy(datafmt.name, "@in_cdr_switch");
	datafmt.namelen = CS_NULLTERM;
	datafmt.datatype = CS_CHAR_TYPE;
	datafmt.maxlength = CS_UNUSED;
	datafmt.status = CS_INPUTVALUE;
	datafmt.locale = NULL;
	if ( ct_param(cmd, &datafmt, (CS_VOID *)par_cdr_switch, CS_NULLTERM, CS_UNUSED) != CS_SUCCEED )
	{
		ret=1;
                err_log("do_proc_after_insert: ct_param 2 fail\n");
                goto Exit_Pro;
	}

	memset(&datafmt, 0, sizeof (datafmt));
	strcpy(datafmt.name, "@in_file_id");
	datafmt.namelen = CS_NULLTERM;
	datafmt.datatype = CS_CHAR_TYPE;
	datafmt.maxlength = CS_UNUSED;
	datafmt.status = CS_INPUTVALUE;
	datafmt.locale = NULL;
	if ( ct_param(cmd, &datafmt, (CS_VOID *)par_file_id, CS_NULLTERM, CS_UNUSED) != CS_SUCCEED )
	{
		ret=1;
                err_log("do_proc_after_insert: ct_param 3 fail\n");
                goto Exit_Pro;
	}

	memset(&datafmt, 0, sizeof (datafmt));
	strcpy(datafmt.name, "@out_rec");
	datafmt.namelen = CS_NULLTERM;
	datafmt.datatype = CS_INT_TYPE;
	datafmt.maxlength = CS_SIZEOF(CS_INT);
	datafmt.status = CS_RETURN;
	datafmt.locale = NULL;

	if ( ct_param(cmd, &datafmt, (CS_VOID *)o_db_rec_num, CS_SIZEOF(CS_INT), CS_UNUSED) != CS_SUCCEED )
	{
		ret=1;
                err_log("do_proc_after_insert: ct_param 4 fail\n");
                goto Exit_Pro;
	}
	
	if( ct_send(cmd) != CS_SUCCEED )
	{
		ret=1;
                err_log("do_proc_after_insert: ct_send fail\n");
                goto Exit_Pro;
	}
/*--------------------------------*/
	/*
	** Process the results of the RPC.
	*/
	while ((retcode = ct_results(cmd, &res_type)) == CS_SUCCEED)
	{
		switch ((int)res_type)
		{
		case CS_ROW_RESULT:
		case CS_STATUS_RESULT:
			/* 
			** Print the result header based on the result type.
			*/
			switch ((int)res_type)
			{
				case  CS_ROW_RESULT:
					fprintf(stdout, "\nROW RESULTS\n");
					break;


				case  CS_STATUS_RESULT:
					fprintf(stdout, "\nSTATUS RESULTS\n");
					break;
			}
	
			/*
			** All three of these result types are fetchable.
			** Since the result model for rpcs and rows have
			** been unified in the New Client-Library, we
			** will use the same routine to display them
			*/
			retcode = ex_fetch_data(cmd,NULL);
			if (retcode != CS_SUCCEED)
			{
				err_log("do_proc_after_insert: ex_fetch_data() failed");
				ret=1;
				goto Exit_Pro;
			}
			break;
		case CS_PARAM_RESULT:
			fprintf(stdout, "\nPARAMETER RESULTS\n");
			retcode = ex_fetch_data(cmd,o_db_rec_num);
			if (retcode != CS_SUCCEED)
			{
				err_log("do_proc_after_insert: ex_fetch_data() failed");
				ret=1;
				goto Exit_Pro;
			}


					
			break;
		case CS_MSG_RESULT:
			/*
			** 
			*/
			retcode = ct_res_info(cmd, CS_MSGTYPE,
					(CS_VOID *)&msg_id, CS_UNUSED, NULL);
			if (retcode != CS_SUCCEED)
			{
				err_log("do_proc_after_insert: ct_res_info(msgtype) failed");
				ret=1;
				goto Exit_Pro;
			}
			fprintf(stdout, "ct_result returned CS_MSG_RESULT where msg id = %d.\n", msg_id);
			break;

		case CS_CMD_SUCCEED:
			/*
			** This means no rows were returned.
			*/
			break;

		case CS_CMD_DONE:
			/*
			** Done with result set.
			*/
			break;

		case CS_CMD_FAIL:
			/*
			** The server encountered an error while
			** processing our command.
			*/
			err_log("do_proc_after_insert: ct_results returned CS_CMD_FAIL.");
			ret=1;
			goto Exit_Pro;
			break;

		default:
			/*
			** We got something unexpected.
			*/
			err_log("do_proc_after_insert: ct_results returned unexpected result type");
			ret=1;
			goto Exit_Pro;
			break;
		}
	}

	switch ((int)retcode)
	{
		case CS_END_RESULTS:
			/*
			** Everything went fine.
			*/
			break;

		case CS_FAIL:
			/*
			** Something failed happened.
			*/
			err_log("do_proc_after_insert: ct_results() failed");
			ret=1;
			goto Exit_Pro;
			break;

		default:
			/*
			** We got an unexpected return value.
			*/
			err_log("do_proc_after_insert: ct_results returned unexpected result type");
			ret=1;
			goto Exit_Pro;
			break;
		}
	
Exit_Pro:
	if(cmd!=NULL)
	{
		if( ct_cmd_drop(cmd) != CS_SUCCEED )
		{
			err_log("do_proc_after_insert: cd_cmd_drop fail\n");
			ret=1;
		}
	}

	return ret;
}
